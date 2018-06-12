using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Security.Cryptography;

namespace Server
{
    class Program
    {
        static void Main(string[] args)
        {
            // Create a new socket object
            var socket = new TcpListener(IPAddress.Any, 8080);
            socket.Start();

            Console.WriteLine("Waiting for connection..");

            // Create serviceprovider-objects
            var rsa = new RSACryptoServiceProvider();
            var aes = new AesCryptoServiceProvider();

            // Encryptor object containing the key and IV
            var encryptor = aes.CreateEncryptor(aes.Key, aes.IV); // Creating the aes key and aes IV

            // Read the file
            string path = @"\\smb\aei034\Desktop\Picture.png";

            while (true)    // What to do when connected
            {
                var connection = socket.AcceptTcpClient(); // Accept incoming connection request
                Console.WriteLine("Connection Accepted!\n");

                NetworkStream stream = connection.GetStream(); //A networkstream for reading and writing

                byte[] pubKeySize = new byte[sizeof(int)]; // Bytearray for storing of pubKeySize from client

                int keySizeInBytes = stream.Read(pubKeySize, 0, sizeof(int)); // Read the pubKeySize
                int rsa_pubKeySize = BitConverter.ToInt32(pubKeySize, 0);   // make pubKeySize an int

                byte[] pubKey = new byte[rsa_pubKeySize]; // Where to store incoming pubKey
                int keyInBytes = stream.Read(pubKey, 0, pubKey.Length); // Reading the public key from client
                
                rsa.ImportCspBlob(pubKey); // Import the rsa key information

                // Encrypt the AES key with the RSA public key
                byte[] encrypted_Rsa = rsa.Encrypt(aes.Key, true);
                byte[] encrypted_Rsa_Size = BitConverter.GetBytes(encrypted_Rsa.Length);
                // Encrypt the AES IV with the RSA public key
                byte[] encrypted_IV = rsa.Encrypt(aes.IV, true);
                byte[] encrypted_IV_Size = BitConverter.GetBytes(encrypted_IV.Length);


                stream.Write(encrypted_Rsa_Size, 0, encrypted_Rsa_Size.Length); // Sending the size of the encrypted AES key to the client
                stream.Write(encrypted_Rsa, 0, encrypted_Rsa.Length);  // Sending the encrypted AES key 

                stream.Write(encrypted_IV_Size, 0, encrypted_IV_Size.Length); // Sending size of IV
                stream.Write(encrypted_IV, 0, encrypted_IV.Length); // Sending the IV

                long fileSize = System.IO.File.OpenRead(path).Length; // size of file, it's a long
                byte[] fileBuffer = BitConverter.GetBytes(fileSize); // Creating a filebuffer for the filesize
                stream.Write(fileBuffer, 0, fileBuffer.Length); // Sending the size of the file to the client


                using (var fileStream = System.IO.File.OpenRead(path))
                {
                    using (var cryptoStream = new CryptoStream(stream, encryptor, CryptoStreamMode.Write))
                        fileStream.CopyTo(cryptoStream);
                }
                Console.WriteLine("The file is now sent");
            }
        }
    }
}
