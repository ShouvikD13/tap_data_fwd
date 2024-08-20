package models

import (
	"log"

	"gopkg.in/ini.v1"
)

/**********************************************************************************/
/*                                                                                 */
/*  Description       : This program initializes a process space configuration     */
/*                      by reading values from a specified section of an INI file. */
/*                      The configuration values are stored in a global map,       */
/*                      allowing easy retrieval of values based on keys.           */
/*                                                                                 */
/*  Functions		  :                                 					       */
/*                        - InitProcessSpace: Loads the INI file, retrieves the    */
/*                          specified section, and stores key-value pairs in the   */
/*                          configMap. Validates the number of tokens against      */
/*                          MaxToken limit.                                        */
/*											                                       */
/*                        - GetProcessSpaceValue: Fetches the value associated     */
/*                          with a given key (token) from the configMap.           */
/*                                                                                 */
/*  Constants         :                                                            */
/*                        - MaxToken: The maximum number of tokens allowed.        */
/*                        												           */
/*                                                                                 */
/**********************************************************************************/

const (
	MaxToken = 50
)

type EnvironmentManager struct {
	ServiceName string
	FileName    string
	cfg         *ini.File
}

func NewEnvironmentManager(serviceName, fileName string) *EnvironmentManager {
	return &EnvironmentManager{
		ServiceName: serviceName,
		FileName:    fileName,
	}
}
func (Em *EnvironmentManager) LoadIniFile() int {
	cfg, err := ini.Load(Em.FileName)
	if err != nil {
		log.Printf("[%s] Error loading INI file: %s, Error: %v", Em.ServiceName, Em.FileName, err)
		return -1
	}
	log.Printf("[%s] Successfully loaded INI file: %s", Em.ServiceName, Em.FileName)
	Em.cfg = cfg
	return 0
}

func (Em *EnvironmentManager) GetProcessSpaceValue(ProcessName, tokenName string) string {
	log.Printf("[%s] Initializing process space", Em.ServiceName)

	section, err := Em.cfg.GetSection(ProcessName)
	if err != nil {
		log.Printf("[%s] Section '%s' not specified in INI file: %s, Error: %v", Em.ServiceName, ProcessName, Em.FileName, err)
		return ""
	}
	log.Printf("[%s] Successfully retrieved section: %s from INI file: %s", Em.ServiceName, ProcessName, Em.FileName)

	key, err := section.GetKey(tokenName)
	if err != nil {
		log.Printf("[%s] Token '%s' not found in section '%s'", Em.ServiceName, tokenName, ProcessName)
		return ""
	}

	value := key.String()
	log.Printf("[%s] Retrieved value for token '%s': %s", Em.ServiceName, tokenName, value)
	return value
}
