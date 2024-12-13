Sure, here are the descriptions for each of the specified functions:

1. *fn_get_fno_pack_seq*
   - *Purpose*: This function retrieves and updates the sequence number for F&O (Futures & Options) pack sequences.
   - *Parameters*:
     - c_ServiceName: A string representing the service name.
     - ptr_st_i_seq: A pointer to a structure containing the sequence information.
     - c_errmsg: A string to store any error messages.
   - *Functionality*: 
     - Logs various levels of debug information.
     - Depending on the request type (c_rqst_typ), it updates the sequence number in the database and retrieves the new sequence number.
     - Returns -1 in case of an error and 0 on success.

2. *fn_sub_seq*
   - *Purpose*: This function is likely responsible for manipulating or retrieving sub-sequences related to orders or transactions.
   - *Parameters*: Not detailed in the extracted content.
   - *Functionality*: Not detailed in the extracted content, but it would typically involve operations similar to managing sub-sequence numbers for transactions.

3. *fn_pack_sprdord_to_nse*
   - *Purpose*: This function is likely responsible for packaging spread orders to be sent to the NSE (National Stock Exchange).
   - *Parameters*: Not detailed in the extracted content.
   - *Functionality*: Not detailed in the extracted content, but it would involve structuring order data in a format required by NSE for spread orders.

4. *fn_pack_ordnry_ord_to_nse*
   - *Purpose*: This function packages ordinary orders to be sent to the NSE.
   - *Parameters*: Not detailed in the extracted content.
   - *Functionality*: Prepares and structures ordinary order data in the format required by NSE for processing.

5. *fn_pack_exer_ord_to_nse*
   - *Purpose*: This function packages exercise orders to be sent to the NSE.
   - *Parameters*: Not detailed in the extracted content.
   - *Functionality*: Prepares and structures exercise order data in the format required by NSE for processing.

6. *fn_pack_ordnry_ord_to_nse_tr*
   - *Purpose*: This function packages ordinary trade orders to be sent to the NSE.
   - *Parameters*: Not detailed in the extracted content.
   - *Functionality*: Prepares and structures ordinary trade order data in the format required by NSE for processing.

For more detailed functionality, parameters, and their types, the specific implementation of each function in the code would need to be reviewed.