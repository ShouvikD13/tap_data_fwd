package models

import "log"

const (
	UPDATION_ON_ORDER_FORWARDING  = "P"
	UPDATION_ON_EXCHANGE_RESPONSE = "R"

	ORDINARY_ORDER = "O"

	FOR_NORM = "N"

	DOWNLOAD     = 10
	NOT_DOWNLOAD = 20
)

/*********************************************************************/
/*                                                                   */
/*  Description : Sets a null character ('\0') at a specified        */
/*                position in a byte slice, marking the end of the   */
/*                string in the slice.                               */
/*                                                                   */
/*********************************************************************/

type FO_Manager struct {
}

func (FOM *FO_Manager) SETNULL(serviceName string, a []byte, length int) {
	if length < len(a) {
		a[length] = 0
		log.Printf("[%s] Set null character at position %d in byte slice", serviceName, length)
	} else {
		log.Printf("[%s] Length %d exceeds byte slice size %d. No action taken.", serviceName, length, len(a))
	}
}
