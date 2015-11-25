#include <pjsua-lib/pjsua.h>

#define PROGRAM_NAME	"SIPCLIENT"

void initialize_pjsua();
void add_transport();
void user_register();
void call_processing();
void make_call();
void add_buddy();
void add_account();
void error_exit(const char *error_msg, pj_status_t action_status);

static void cb_on_incoming_call(pjsua_acc_id account_id, pjsua_call_id call_id, pjsip_rx_data *rdata);
static void cb_on_call_state(pjsua_call_id call_id, pjsip_event *e);
static void cb_on_reg_state2(pjsua_acc_id account_id, pjsua_reg_info *info);
static void cb_on_buddy_state(pjsua_buddy_id buddy_id);

char          SIP_SERVER[20];
char          USER_NAME[20];
char          USER_PASSWORD[20];
pjsua_call_id call_id_global = -1;

/**********************************************************************************************
                   MAIN FUNCTIONALITY OF THE SIP CLIENT PROGRAM 
***********************************************************************************************/
int  main(int argc, char *argv[]) {
     if (argc != 4) {
        printf ("Invalid arguments. Please enter SIP Domain IP address followed by username followed by password." 
                "Example: 169.254.1.4 username password \n");
        return;
     }
     
     strcpy(SIP_SERVER,argv[1]);            //argv[1] corresponds to SIP DOMAIN  
     strcpy(USER_NAME,argv[2]);             //argv[2] corresponds to user account name
     strcpy(USER_PASSWORD,argv[3]);         //argv[3] corresponds to the user password for the account
     
     pj_status_t  action_status;

     action_status = pjsua_create();        //Creating PJSUA
     if (action_status != PJ_SUCCESS) 
        error_exit("Error in the function pjsua_create()", action_status);

     initialize_pjsua();                    //Initialize PJSUA
  
     add_transport();                       //Adding transport for SIP
 
     action_status = pjsua_start();         //Starting PJSUA
     if (action_status != PJ_SUCCESS) 
         error_exit("Error in the function pjsua_start()", action_status);

     add_account();                         //Adding user account
     
     call_processing();                     //All the main processing for client
     
     pjsua_destroy();                       //Destroy PJSUA for shutdown

     return 0;
}

/**********************************************************************************************
                   INITIALIZE PJSUA WITH SPECIFIC SETTINGS
***********************************************************************************************/
void initialize_pjsua() {
     pjsua_config init_config;
     pjsua_logging_config log_config;

     pjsua_config_default(&init_config);
     init_config.cb.on_incoming_call = &cb_on_incoming_call;
     init_config.cb.on_call_state = &cb_on_call_state;
     init_config.cb.on_reg_state2 = &cb_on_reg_state2;
     init_config.cb.on_buddy_state = &cb_on_buddy_state;

     pjsua_logging_config_default(&log_config);
     log_config.console_level = 4;

     pj_status_t action_status = pjsua_init(&init_config, &log_config, NULL);
     if (action_status != PJ_SUCCESS) 
        error_exit("Error in the function pjsua_init()", action_status);
}

/**********************************************************************************************
                   ADD SIP TRANSPORT WITH PORT NUMBER
***********************************************************************************************/
void add_transport() {
     pjsua_transport_config transport_config;
     
     pjsua_transport_config_default(&transport_config);
     transport_config.port = 0;
     
     pj_status_t action_status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &transport_config, NULL);
     if (action_status != PJ_SUCCESS) 
        error_exit("Error creating transport", action_status);
}

/**********************************************************************************************
                   ADD USER ACCOUNT FOR THE DOMAIN FRM ARGUMENTS
***********************************************************************************************/
void add_account() {
     pjsua_acc_config account_config;
     pjsua_acc_id     account_id;
                                   
     pjsua_acc_config_default(&account_config);
     char sip_uri[50];
     snprintf(sip_uri, sizeof(sip_uri), "sip:%s@%s", USER_NAME, SIP_SERVER); 
     char sip[50];
     snprintf(sip, sizeof(sip), "sip:%s", SIP_SERVER); 
     account_config.id         = pj_str(sip_uri);
     account_config.reg_uri    = pj_str(sip);
     account_config.cred_count = 1;
     account_config.cred_info[0].realm     = pj_str("asterisk");
     account_config.cred_info[0].scheme    = pj_str("digest");
     account_config.cred_info[0].username  = pj_str(USER_NAME);
     account_config.cred_info[0].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
     account_config.cred_info[0].data      = pj_str(USER_PASSWORD);
     account_config.register_on_acc_add = PJ_FALSE;
 
     pj_status_t action_status = pjsua_acc_add(&account_config, PJ_TRUE, &account_id);
     if (action_status != PJ_SUCCESS) 
        error_exit("Error in adding account", action_status);    

     action_status = pjsua_acc_set_online_status(0, PJ_TRUE);
     if (action_status != PJ_SUCCESS) 
        error_exit("Error in modifying account presence", action_status); 
}

/**********************************************************************************************
                   ALL MAIN CALL PROCESSING FOR THE CLIENT
***********************************************************************************************/
void call_processing() {
     while (1) {
        char        choice;
        pj_status_t action_status;
 
        printf("Enter 'r' to register; 'u' to unregister; 'b' to add buddy;" 
               "'m' to make a call; 'h' to hangup a call and 'q' to end registration and quit \n");
        scanf("%c",&choice);

        switch (choice) {
          case 'q':        
               return;

          case 'r':
               action_status = pjsua_acc_set_registration(0, PJ_TRUE);
               if (action_status != PJ_SUCCESS) 
                  error_exit("Error in account unregistration", action_status);    
               break;

          case 'u':
               action_status = pjsua_acc_set_registration(0, PJ_FALSE);
               if (action_status != PJ_SUCCESS) 
                  error_exit("Error in account unregistration", action_status);    
               break;
  
          case 'h':
               pjsua_call_hangup_all();
               break;

          case 'm': 
               make_call();
               break;

          case 'a': 
               pjsua_call_answer(call_id_global, 200, NULL, NULL);      //Answer incoming calls with 200 OK
               break;

          case 'c':
               pjsua_call_answer(call_id_global, 486, NULL, NULL);      //Answer incoming calls with 486 Busy
               break;

          case 'b':
               add_buddy();
               break;
        }
     }
} 

/**********************************************************************************************
                   MAKE A CALL TO DESIRED DESTINATION
***********************************************************************************************/
void make_call() {
     char input_uri[40];
     pjsua_acc_id account_id;
     printf("Enter the destination URI \n");
     scanf("%s", input_uri);
     printf("uri %s \n", input_uri);

     pj_status_t action_status = pjsua_verify_url(input_uri);
     if (action_status != PJ_SUCCESS) 
        printf("Invalid URL entered. Try again. \n");
     else {
        pj_str_t destination_uri = pj_str(input_uri);
	action_status = pjsua_call_make_call(account_id, &destination_uri, 0, NULL, NULL, NULL);
        if (action_status != PJ_SUCCESS) 
           error_exit("Error making call", action_status);
     } 
}

/**********************************************************************************************
                   ADD BUDDY FROM ANY DESIRED DESTINATION
***********************************************************************************************/
void add_buddy() {
     pjsua_buddy_id buddy_id;
     char input_uri[40];
     
     printf("Enter the destination URI to add as buddy along with port number \n");
     scanf("%s", input_uri);
     printf("buddy uri %s \n", input_uri);

     pj_status_t action_status = pjsua_verify_url(input_uri);
     if (action_status != PJ_SUCCESS) 
        printf("Invalid URL entered. Try again. \n");
     else {
        pj_str_t destination_uri = pj_str(input_uri);
        pjsua_buddy_id destination_bud_id = pjsua_buddy_find(&destination_uri);
	if (destination_bud_id == PJSUA_INVALID_ID) {
           pjsua_buddy_config config;
           pjsua_buddy_config_default(&config);
           config.uri = destination_uri;;
           config.subscribe = 1;
           action_status = pjsua_buddy_add(&config, &buddy_id);
           if (action_status != PJ_SUCCESS)
              error_exit("Error on adding buddy", action_status);
        }
     }
}

/**********************************************************************************************
                   FUNCTION TO HANDLE THE ERRORS AND DESTROY PJSUA
***********************************************************************************************/
void error_exit(const char *error_msg, pj_status_t action_status) {
     pjsua_perror(PROGRAM_NAME, error_msg, action_status);
     pjsua_destroy();
     exit(1);
}

/**********************************************************************************************
                   CALLBACK FUNCTION TO HANDLE INCOMING CALLS
***********************************************************************************************/
static void cb_on_incoming_call(pjsua_acc_id account_id, pjsua_call_id call_id, pjsip_rx_data *rdata) {
    pjsua_call_info call_info;
    char            choice;
   
    if (pjsua_call_get_count() > 1) {
       pjsua_call_answer(call_id, 486, NULL, NULL);      //Answer incoming calls with 486 Busy
       return;
    }

    PJ_UNUSED_ARG(account_id);
    PJ_UNUSED_ARG(rdata);

    pjsua_call_get_info(call_id, &call_info);
    call_id_global = call_id;
    PJ_LOG(3,(PROGRAM_NAME, "Incoming call from %.*s",
                         (int)call_info.remote_info.slen,
                         call_info.remote_info.ptr));
    
    printf("Enter 'a' to answer the call and 'c' to reject \n");
    scanf("%c", &choice);
}

/**********************************************************************************************
                   CALLBACK FUNCTION TO CAPTURE CALL STATUS
***********************************************************************************************/
static void cb_on_call_state(pjsua_call_id call_id, pjsip_event *e) {
    pjsua_call_info call_info;

    PJ_UNUSED_ARG(e);

    pjsua_call_get_info(call_id, &call_info);
 
    PJ_LOG(3,(PROGRAM_NAME, "Call %d state=%.*s", call_id,
                         (int)call_info.state_text.slen,
                         call_info.state_text.ptr));
}

/**********************************************************************************************
                   CALLBACK FUNCTION TO CAPTURE REGISTRATION STATUS
***********************************************************************************************/
static void cb_on_reg_state2(pjsua_acc_id account_id, pjsua_reg_info *info) {
    PJ_UNUSED_ARG(account_id);

    if (info != NULL && info->cbparam != NULL) 
      PJ_LOG(3,(PROGRAM_NAME, "Expiration=%d",info->cbparam->expiration));
}

/**********************************************************************************************
                   CALLBACK FUNCTION FUNCTION TO CAPTURE BUDDY STATUS
***********************************************************************************************/
static void cb_on_buddy_state(pjsua_buddy_id buddy_id) {
    pjsua_buddy_info bi;

    pjsua_buddy_get_info(buddy_id , &bi);

    PJ_LOG(3,(PROGRAM_NAME, "Buddy %.*s with id %d is %.*s", 
                            (int)bi.uri.slen, bi.uri.ptr, bi.id, 
                            (int)bi.status_text.slen, bi.status_text.ptr));
}
