package config

import (
	"encoding/json"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

type Config struct {
	VisPort             int
	VisPollInterval     string
	VisWaitForNodeStart string
	NodeLowPort         int
	NodeHighPort        int
	Hosts               []string
	MaxRun              string
}

// Returns host name, stripping the uvrocks ".local" suffix
//
// Compute nodes on a Rocks cluster add a ".local" to their hostname. If you
// start a server on "compute-1-1", it will report that its hostname is
// "compute-1-1.local". This can get confusing, so we strip that suffix here.
//
func Hostname() string {
	hostname, _ := os.Hostname()
	hostname = strings.TrimSuffix(hostname, ".local")
	return hostname
}

// Read a config, or write a default config for editing
func ReadConfig() (Config, error) {

	// Default config
	conf := Config{
		VisPort:             8182,
		VisPollInterval:     "200ms",
		VisWaitForNodeStart: "1000ms",
		NodeLowPort:         9000,
		NodeHighPort:        9999,
		Hosts:               computeNodes(),
		MaxRun:              "20m",
	}

	execpath, err := filepath.Abs(os.Args[0])
	if err != nil {
		return conf, err
	}
	execdir := filepath.Dir(execpath)
	parent := filepath.Dir(execdir)
	configpath := parent + "/chord-config.json"

	f, err := os.Open(configpath)
	if err != nil {
		log.Println("No config, creating: " + configpath)

		f, err := os.Create(configpath)
		if err != nil {
			return conf, err
		}

		encoder := json.NewEncoder(f)
		encoder.SetIndent("", "    ")
		encoder.Encode(conf)
		f.Close()

		log.Println(conf)
		return conf, nil
	}

	log.Println("Reading config from " + configpath)

	decoder := json.NewDecoder(f)
	err = decoder.Decode(&conf)
	if err != nil {
		return conf, err
	}
	f.Close()

	log.Println(conf)
	return conf, nil
}

func computeNodes() []string {

	cmdline := []string{"bash", "-c",
		"rocks list host compute | cut -d: -f1 | tail -n +2",
	}
	cmd := exec.Command(cmdline[0], cmdline[1:]...)
	bytes, err := cmd.Output()
	if err != nil {
		log.Printf("Error getting compute nodes. Using hostname (%s): %s,", Hostname(), err)
		return []string{Hostname()}
	}

	str := strings.TrimSpace(string(bytes))
	nodes := []string{}
	if str != "" {
		nodes = strings.Split(str, "\n")
	} else {
		log.Printf("No compute nodes. Using hostname (%s)", Hostname())
		return []string{Hostname()}
	}

	return nodes
}
