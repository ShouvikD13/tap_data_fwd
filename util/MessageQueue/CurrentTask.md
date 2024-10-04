Now i have to create the logger file .
Requirements for the log file 

    1. There will be two levels there in the log file . (INFO , ERROR)
    2. The format of log will be like this "Level , [Timestamp] , [Service Name] , [Function Name] , Message , Values ... "
        To do this : we have to write a function which will set the format by using 'sprintf' or 'snprintf'.

        void write_log(char * servicename , char * function_name , char *  Message , ... )
        // there can be many parameter present so i have to use varargs.  

    3. Logs should be generated in both terminal and the File.
    4. File Should be named as `ULOG.2024-10-03.log`. 
    5. there should be only one log file for one day .
    6. Log file Rotation . "Write Appropriate Rotation Logic".

    ---------------------------------------------------------------------
    Little bit about the usage of "Variable Length Argument in C".
    there are four methods used in `Variable Length Argument in C`
    1. `va_list` this is a list which will hold all the variable . we have to initialize this list .
    2. `va_start` this will initialise `va_list` . but we have to specify the last fixed argument which is `message` in this     case. this method will allocate the next address after the `message`.
    3. `va_args` this 

