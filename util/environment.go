package util

import (
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
	ServiceName   string
	FileName      string
	cfg           *ini.File
	LoggerManager *LoggerManager
}

func NewEnvironmentManager(serviceName, fileName string, LM *LoggerManager) *EnvironmentManager {
	return &EnvironmentManager{
		ServiceName:   serviceName,
		FileName:      fileName,
		LoggerManager: LM,
	}
}
func (Em *EnvironmentManager) LoadIniFile() int {
	cfg, err := ini.Load(Em.FileName)
	if err != nil {
		Em.LoggerManager.LogError(Em.ServiceName, " Error loading INI file: %s, Error: %v", Em.FileName, err)
		return -1
	}
	Em.LoggerManager.LogInfo(Em.ServiceName, "Successfully loaded INI file: %s", Em.FileName)
	Em.cfg = cfg
	return 0
}

func (Em *EnvironmentManager) GetProcessSpaceValue(ProcessName, tokenName string) string {
	Em.LoggerManager.LogInfo(Em.ServiceName, "Initializing process space")

	section, err := Em.cfg.GetSection(ProcessName)
	if err != nil {
		Em.LoggerManager.LogError(Em.ServiceName, "[GetProcessSpaceValue] Section '%s' not specified in INI file: %s, Error: %v", ProcessName, Em.FileName, err)
		return ""
	}
	Em.LoggerManager.LogInfo(Em.ServiceName, "[GetProcessSpaceValue] Successfully retrieved section: %s from INI file: %s", ProcessName, Em.FileName)
	key, err := section.GetKey(tokenName)
	if err != nil {
		Em.LoggerManager.LogError(Em.ServiceName, "[GetProcessSpaceValue] Token '%s' not found in section '%s'", tokenName, ProcessName)

		return ""
	}
	value := key.String()

	Em.LoggerManager.LogInfo(Em.ServiceName, "[GetProcessSpaceValue] Retrieved value for token '%s': %s", tokenName, value)

	return value
}
