// Credits: Edvard Pedersen

package main

import(
  "os"
  "log"
  "bufio"
//  "io"
  "strings"
  "fmt"
)

func main() {
  args := os.Args;
  fmt.Printf("File to open: %s\n", args[1]);
  file, err := os.Open(args[1]);
  if err != nil {
    log.Fatal(err);
  }

  entry := make(map[string]string);
  
  scanner := bufio.NewScanner(file);
  for scanner.Scan() { //Scan the file
    retText := scanner.Text();
    if strings.HasPrefix(retText, "//") {
      doSomething(entry); //Entry is complete, do something with it
      entry = make(map[string]string); //Start new entry
    } else {
      entry[retText[0:2]] += retText[5:];
    }
  }

  file.Close();
}

//Do something with the entry (stored in a map)
func doSomething(entry map[string]string) {
  fmt.Println("Keys:");
  for key, _ := range entry {
    fmt.Printf("%s:", key);
  }
  fmt.Printf("\n");
  return;
}
