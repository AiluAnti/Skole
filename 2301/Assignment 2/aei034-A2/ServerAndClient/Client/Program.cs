using System;
using System.Net.Sockets;
using System.Security.Cryptography;

namespace Client
{
    class Program
    {
        static void Main(string[] args)
        {
            var rsa = new RSACryptoServiceProvider();
            var aes = new AesCryptoServiceProvider();

            byte[] pubKey = rsa.ExportCspBlob(false); // Creating an assymetric RSA key
            byte[] pubKeySize = BitConverter.GetBytes(pubKey.Length); // Size of pubKey in bytes



            TcpClient tcpclnt = new TcpClient();
            Console.WriteLine("Connecting.....");

            tcpclnt.Connect("127.0.0.1", 8080);
            // use the ipaddress as in the server program

            NetworkStream stm = tcpclnt.GetStream();

            stm.Write(pubKeySize, 0, pubKeySize.Length); // Sending the pubKeySize

            stm.Write(pubKey, 0, pubKey.Length); // Sending the pubKey
            // Pubkeysize and pubkey has been sent to the server

            byte[] encrypted_Aes = new byte[sizeof(int)];   // Bytearray for the encrypted AES key size
            stm.Read(encrypted_Aes, 0, encrypted_Aes.Length);   // Read the encrypted AES key size from server
            // The encrypted AES key size has been recieved by the client

            int encrypted_Aes_Size = BitConverter.ToInt32(encrypted_Aes, 0);    // The int size of the key

            byte[] encrypted_Aes_Key = new byte[encrypted_Aes_Size];
            stm.Read(encrypted_Aes_Key, 0, encrypted_Aes_Key.Length);
            // The encrypted AES key has now been recieved by the client

            byte[] encrypted_IV_Size = new byte[sizeof(int)]; // Bytearray for the encrypted IV size
            stm.Read(encrypted_IV_Size, 0, encrypted_IV_Size.Length); // Read the size

            int IV_Size = BitConverter.ToInt32(encrypted_IV_Size, 0);
            // The encrypted IV Size has now been recieved

            byte[] encrypted_IV = new byte[IV_Size];
            stm.Read(encrypted_IV, 0, encrypted_IV.Length);
            // The encrypted IV has noe been recieved
            
            // Time to recieve the encrypted file from the server
            byte[] filesize = new byte[sizeof(long)];
            stm.Read(filesize, 0, filesize.Length);

            byte[] decryptedKey = rsa.Decrypt(encrypted_Aes_Key, true);
            byte[] decryptedIV = rsa.Decrypt(encrypted_IV, true);
            ICryptoTransform decryptor = aes.CreateDecryptor(decryptedKey, decryptedIV);

            using (var fileStream = System.IO.File.Create(@"\\smb\aei034\Desktop\nyFil.png"))
            {
                using (var cryptoStream = new CryptoStream(stm, decryptor, CryptoStreamMode.Read))
                {
                    cryptoStream.CopyTo(fileStream);
                }
            }
            Console.WriteLine("The file is now recieved");
            tcpclnt.Close();

        }
    }
}
