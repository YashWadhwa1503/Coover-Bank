#include "cJSON.h"
#include <ctype.h>
// #include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#define MAX_USERS 100
#define MAX_USERNAME_LENGTH 30
#define MAX_PASSWORD_LENGTH 30
#define MAX_NAME_LENGTH 50
#define MAX_ACCOUNT_NAMES_LENGTH 30
#define MAX_ACCOUNTS 10

#define DATABASE_FILE "database.json"

typedef struct {
  char Username[MAX_USERNAME_LENGTH];
  char Password[MAX_PASSWORD_LENGTH];
  char Name[MAX_NAME_LENGTH];
  double account_balances[MAX_ACCOUNTS];
  char account_names[MAX_ACCOUNTS][MAX_ACCOUNT_NAMES_LENGTH];
  char account_types[MAX_ACCOUNTS][MAX_ACCOUNT_NAMES_LENGTH];
  int num_accounts;
} user_info;

user_info database[MAX_USERS]; // Definition of the main struct used for all data storage
int num_users = 0;

//===============================================
//			Function used to format a string to all lowercase
//===============================================

void format_string(const char *in_string, char *out_string, size_t max_length) {
	size_t i;
  for ( i = 0; in_string[i] != '\0' && i < max_length; i++) {
    out_string[i] = tolower(in_string[i]);
  }
  out_string[i] = '\0';
}

//===============================================
// Add a user into the database struct for short term
//                     Storage
//===============================================

void add_user(char *name, char *username, char *password) {
  if (num_users < MAX_USERS) {

    strncpy(database[num_users].Name, name, MAX_NAME_LENGTH);

    char lowercase_username[MAX_USERNAME_LENGTH];
    format_string(username, lowercase_username, MAX_USERNAME_LENGTH);
    strncpy(database[num_users].Username, lowercase_username,
            MAX_USERNAME_LENGTH);

    strncpy(database[num_users].Password, password, MAX_PASSWORD_LENGTH);
 
  } else {
    printf("Database full, cannot add record.\n");
  }
}

//===============================================
// Add an account into the database struct for short term
//                     storage
//===============================================

void add_account(user_info *user_info, const int account_type,int initial_balance, const int user_info_index, const char *account_name) {
  if (user_info->num_accounts >= MAX_ACCOUNTS) {
    printf("\nAccount already has maximum number of accounts.\n");
    return;
  }

  if (account_type == 1) { // CHECKINGS ACCOUNT
    strncpy(database[user_info_index].account_types[database[user_info_index].num_accounts],"Checkings", MAX_ACCOUNT_NAMES_LENGTH);
    strncpy(database[user_info_index].account_names[database[user_info_index].num_accounts],account_name, MAX_ACCOUNT_NAMES_LENGTH);

  } else if (account_type == 2) { // SAVINGS ACCOUNT
    strncpy(database[user_info_index]
                .account_types[database[user_info_index].num_accounts],
            "Savings", MAX_ACCOUNT_NAMES_LENGTH);
    strncpy(database[user_info_index]
                .account_names[database[user_info_index].num_accounts],
            account_name, MAX_ACCOUNT_NAMES_LENGTH);
  }

		// setting the new user account's balance and incrementing the num accounts 
  
  double balance = initial_balance;
  database[user_info_index].account_balances[database[user_info_index].num_accounts] = balance;
  database[user_info_index].num_accounts++;
}

//===============================================
//			Save the entire database struct in the database json for longterm storage
//===============================================

void save_database(char *filename) {
  cJSON *root = cJSON_CreateArray();

  for (int i = 0; i < num_users; i++) {
    cJSON *user_info = cJSON_CreateObject();
    cJSON_AddStringToObject(user_info, "name", database[i].Name);

    char lowercase_username[MAX_USERNAME_LENGTH];
    format_string(database[i].Username, lowercase_username,
                  MAX_USERNAME_LENGTH);

    cJSON_AddStringToObject(user_info, "username", lowercase_username);

    cJSON_AddStringToObject(user_info, "password", database[i].Password);

    cJSON *accounts = cJSON_CreateArray();

    for (int j = 0; j < database[i].num_accounts; j++) {
      cJSON *account = cJSON_CreateObject();

      cJSON_AddStringToObject(account, "name", database[i].account_names[j]);
      cJSON_AddStringToObject(account, "type", database[i].account_types[j]);
      cJSON_AddNumberToObject(account, "balance",
                              database[i].account_balances[j]);
      cJSON_AddItemToArray(accounts, account);
    }
		
    cJSON_AddItemToObject(user_info, "accounts", accounts);
    cJSON_AddItemToArray(root, user_info);
  }

  char *json_string = cJSON_Print(root);
  FILE *fp = fopen(filename, "w");
  fprintf(fp, "%s", json_string);
  fclose(fp);
  free(json_string);
}

//===============================================
//			loads the entire database json into memory in the database struct
//===============================================

void load_database(char *filename) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Error. Could not open file %s", filename);
    return;
  }

  char *json_string = NULL;
  size_t json_size = 0;
  char buffer[4096];

  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    json_size += strlen(buffer);
    json_string = realloc(json_string, json_size + 1);
    strcat(json_string, buffer);
  }
	
  fclose(fp);
  cJSON *root = cJSON_Parse(json_string);
  free(json_string);

  num_users = cJSON_GetArraySize(root);

  for (int i = 0; i < num_users; i++) {
    cJSON *user_info = cJSON_GetArrayItem(root, i);
    cJSON *name = cJSON_GetObjectItem(user_info, "name");
    cJSON *username = cJSON_GetObjectItem(user_info, "username");
    cJSON *password = cJSON_GetObjectItem(user_info, "password");

    strcpy(database[i].Name, name->valuestring);
    strcpy(database[i].Username, username->valuestring);
    strcpy(database[i].Password, password->valuestring);

    cJSON *accounts = cJSON_GetObjectItem(user_info, "accounts");
    int num_accounts = cJSON_GetArraySize(accounts);

    for (int j = 0; j < num_accounts; j++) {
      cJSON *account = cJSON_GetArrayItem(accounts, j);
      cJSON *account_name = cJSON_GetObjectItem(account, "name");
      cJSON *account_type = cJSON_GetObjectItem(account, "type");
      cJSON *account_balance = cJSON_GetObjectItem(account, "balance");

      strcpy(database[i].account_names[j], account_name->valuestring);
      strcpy(database[i].account_types[j], account_type->valuestring);
      database[i].account_balances[j] = account_balance->valuedouble;
    }
    database[i].num_accounts = num_accounts;
  }
  cJSON_Delete(root);
}

//===============================================
//			main menu
//===============================================

int main_menu() {
	
  printf("\n\033[0;31m======================\n");
  printf("\nWelcome to Coover Bank!\n\n");
  printf("1. Create an account\n");
  printf("2. Login to account\n");
  printf("\nAt any point type in \"123\" to exit\n");
  printf("\n======================\033[0m\n");
  printf("\n\033[32mEnter selection: \033[0m");
	
	int menu_option;
  scanf("%d", &menu_option);
  return menu_option;
}

//===============================================
//			create a new checkings or savings account
//===============================================

int checkings_savings_screen(int user_info_index,char account_name[MAX_ACCOUNT_NAMES_LENGTH],int *initial_balance) 
{
	int menu_optionCS;
	
  printf("\nSelect what type of account to create.\n\n	1.) Checkings\n	2.) "
         "Savings\n\nOtherwise type in \"123\" to return: ");
  scanf("%d", &menu_optionCS);

  if (menu_optionCS == 1) { // Account type is checkings
    // SEE MAIN FOR CREATING CHECKINGS ACCOUNT
    // LOGIC IN MAIN FUNCTION

    printf("\nWhat is the name of your account?: ");
    scanf("%s", account_name);

    printf("What is the initial deposit amount?: ");
    scanf("%d", initial_balance);

    return 1;
  } else if (menu_optionCS == 2) { // Account type is savings
    // SEE MAIN FOR CREATING SAVINGS ACCOUNT

    printf("\nWhat is the name of your account?: ");
    scanf("%s", account_name);

    printf("What is the initial deposit amount?: ");
    scanf("%d", initial_balance);

    return 2;
  } else if (menu_optionCS == 123) {
    // exit to accounts menu
    system("clear");
    return 123;
  } else {
    // invalid menu option
    printf("invalid menu option\n");
    return 0;
  }
}

//===============================================
//			login screen prompts the user for
//===============================================

int login_screen(char username[MAX_USERNAME_LENGTH],char password[MAX_PASSWORD_LENGTH]) {

  printf("\nEnter username: ");
  scanf("%s", username);
	
  printf("Enter password: ");
  scanf("%s", password);
}

//===============================================
//			create a new user account in the first screen of the app
//===============================================

int create_account() { // LOGIC (NOT SAME AS ADD_ACCOUNT)
  char username[MAX_USERNAME_LENGTH];
  char password[MAX_PASSWORD_LENGTH];
  char name[MAX_NAME_LENGTH];
  int exit_code = 1;
  int fail_code = 10;
  printf("\nType in \"123\" to cancel.\n");

  getchar();
  printf("\nEnter your name: ");
  fgets(name, MAX_NAME_LENGTH, stdin);

  if (strcmp(name, "123\n") == 0) {
    exit_code = 0;
    system("clear");
    goto exit;
  }

  printf("Enter new username: ");
  fgets(username, MAX_NAME_LENGTH, stdin);

  if (strcmp(username, "123\n") == 0) {
    exit_code = 0;
    system("clear");
    goto exit;
  }

  printf("Enter new password: ");
  fgets(password, MAX_NAME_LENGTH, stdin);

  if (strcmp(password, "123\n") == 0) {
    exit_code = 0;
    system("clear");
    goto exit;
  }
char account_name[MAX_ACCOUNT_NAMES_LENGTH];
    printf("\nYour first account will be a checkings account.\n");
    sleep(2.5);
	  printf("\nWhat would you like to call this checkings account?: ");
    scanf("%s", account_name);

  int initial_balance;

  printf("\nEnter initial balance (needs to be at least $100): ");
  scanf("%d", &initial_balance);

  if (initial_balance < 100) {
    goto fail;
  } else {

  name[strcspn(name, "\n")] = '\0';
  username[strcspn(username, "\n")] = '\0';
  password[strcspn(password, "\n")] = '\0';

  char lowercase_username[MAX_USERNAME_LENGTH];
  format_string(username, lowercase_username, MAX_USERNAME_LENGTH);

  add_user(name, lowercase_username, password);

  add_account(database, 1, initial_balance, num_users, account_name);
  num_users++;
  // return login_screen();
  save_database(DATABASE_FILE);
exit:
  return exit_code;
fail:
  return fail_code;
  }
}

//===============================================
//			Main account screen for the banking app
//===============================================

int account_screen(const int user_info_index) {
  printf("\n\033[34mWelcome back %s!\033[0m\n", database[user_info_index].Name);

  printf("\n\033[38;2;255;165;0m=== Accounts ===\n\n");
	for (int i = 0; i < database[user_info_index].num_accounts; i++) {
    printf("%s Account: %s Balance: %.2lf\n\n",
					 database[user_info_index].account_types[i],
           database[user_info_index].account_names[i],
           database[user_info_index].account_balances[i]);
  }
  printf("================\033[0m\n\n");

  int menu_optionACS;
  printf("Select an option.\n\n");
  printf(
      "1.) Create a checkings or savings account\n2.) Transfer money\n3.) Withdraw or deposit\n4.) "
      "Interest calculator\n\nOtherwise type in \"123\" to logout.\n\n\033[32mEnter selection: \033[0m");
  scanf("%d", &menu_optionACS);

  if (menu_optionACS == 1) {
    // redirect to checkings_savings_screen
    return 1;
  } else if (menu_optionACS == 2) {
    // redirect to transfer money screen
    return 2;
  } else if (menu_optionACS == 3) {
    // redirect to deposit/withdraw screen
    return 3;
  } else if (menu_optionACS == 4) {
    // redirect to interest calculator screen
    return 4;
  } else if (menu_optionACS == 123) {
    // exit to menu menu
    printf("\n\033[34mGoodbye.\033[0m\n");
		sleep(2);
    system("clear");
    return 123;
  } else {
    // invalid input
    printf("\nInvalid input.\n");
    sleep(2);
    system("clear");
    account_screen(user_info_index);
  }
}

double ir_calc(double principle, double amount, double rate, int time) {
  double final_amount;
  amount = amount / 100;
  final_amount = principle * pow((1.0 + (amount / rate)), rate * time);
  
  return final_amount;
  }

//===============================================
//			Transfer money from one account to another
//===============================================

void transfer_money(const int user_info_index) 
{
  int transfer_from_selection;
  int transfer_in_selection;
  double amount_to_transfer;

  printf("\nYou have selected to transfer money.\n");
  
  printf("\nFrom which account? \n");

  printf("\n\033[38;2;255;165;0m=== Accounts ===\n\n");
	for (int i = 0; i < database[user_info_index].num_accounts; i++) {
    printf("Account %d: %s Balance: %.2lf\n\n",
           i + 1,
           database[user_info_index].account_names[i],
           database[user_info_index].account_balances[i]);
  }
  printf("================\033[0m\n\n");
  printf("\033[32mEnter selection: \033[0m");
  scanf("%d", &transfer_from_selection);

  printf("\nTo which account? \n");
  printf("\n\033[38;2;255;165;0m=== Accounts ===\n\n");
	for (int i = 0; i < database[user_info_index].num_accounts; i++) {
    printf("Account %d: %s Balance: %.2lf\n\n",
           i + 1,
           database[user_info_index].account_names[i],
           database[user_info_index].account_balances[i]);
  }
  printf("================\033[0m\n"); 

  printf("\n\033[32mEnter selection: \033[0m");
  scanf("%d", &transfer_in_selection);

  if (transfer_from_selection == transfer_in_selection) 
  {
	  printf("\nInvalid transfer: Cannot transfer into same account.\n");
	  sleep(3);
	  system("clear");
  } else if (transfer_from_selection > database[user_info_index].num_accounts || transfer_in_selection > database[user_info_index].num_accounts) {
		printf("\nInvalid transfer: Cannot find account to transfer to.\n");
	  sleep(3);
	  system("clear");
	}
  else 
  {
  printf("\nAmount to transfer from Account %d to Account %d? ", transfer_from_selection, transfer_in_selection);
  scanf("%lf", &amount_to_transfer); // CAN CHANGE THIS FROM ACCOUNT NUMBERS TO ACCOUNT NAMES
    if (amount_to_transfer > (database[user_info_index].account_balances[transfer_from_selection - 1]))
    {
     printf("\nInvalid transfer: Insufficient funds.\n");
      sleep(2);
      system("clear");
    }
    else if (amount_to_transfer == 0) {
      printf("\nInvalid transfer: Cannot transfer zero funds.\n");
      sleep(2);
      system("clear");
    }
    else if (amount_to_transfer < 0) 
    {
     printf("\nInvalid transfer: Cannot transfer negative funds.\n");
      sleep(2);
      system("clear");
    }
  else 
    {
    (database[user_info_index].account_balances[transfer_from_selection - 1]) -= amount_to_transfer;
  save_database(DATABASE_FILE);
  (database[user_info_index].account_balances[transfer_in_selection - 1]) += amount_to_transfer;
  save_database(DATABASE_FILE);

      printf("\nTransfer successful.\n");

      sleep(3);

      system("clear");
    }  
  }
}

//========================================================
//			Main function doing the brunt work of the logic
//========================================================

int main(void) {
  int menu_option = 0;

  load_database(DATABASE_FILE);

  while (menu_option != 123) {
  MAIN_MENU:;
    menu_option = main_menu();
    if (menu_option == 1) {
      // go to create an account screen
      
      int create_PF = create_account();
      if (create_PF == 1) {
        // create account successful redirect to main menu
        printf("\nAccount created successfully.\n");
        sleep(3);
        system("clear");
        goto MAIN_MENU;
      } else if (create_PF == 10) {
        // create account failed redirect to main menu
        printf("\nInsufficient funds to create bank account.\n");
        sleep(3);
        system("clear");
        goto MAIN_MENU;
      }
    } else if (menu_option == 2) {

      printf("\nYou have selected to login.\n");
      printf("\nYou have three attempts.\n");
      
    LOGIN_SCREEN:;
      // go to login screen
      int login_attempt = 0;
    LOGIN_FAIL:;
      char input_username[MAX_USERNAME_LENGTH];
      char input_password[MAX_PASSWORD_LENGTH];
      int user_info_index;
      int login_PF = login_screen(input_username, input_password);

      char formatted_username[MAX_USERNAME_LENGTH];
      format_string(input_username, formatted_username, MAX_USERNAME_LENGTH);

      for (int i = 0; i <= num_users; i++) {

        if (strcmp(database[i].Username, formatted_username) == 0 &&
            strcmp(database[i].Password, input_password) == 0) {
          user_info_index = i;
          login_PF = 1;
          break;
        }else {
					login_PF = 0;
				}
      }

      if (login_PF == 0) { // if the login fails
        // failed login
        printf("\nIncorrect username or password.\n");
        login_attempt++;
        if (login_attempt <= 2) {
          goto LOGIN_FAIL;
        } else {
          printf("\nAttempt limit reached. Redirecting to main menu...\n");
          sleep(2.5);
          system("clear");
          goto MAIN_MENU;
        }
  } else if (login_PF == 1) {
        // login suceeded go to accounts screen of user
      system("clear");
      ACCOUNT_SCREEN:;
        system("clear");
        int menu_account = account_screen(user_info_index);
        if (menu_account == 1) {  // CREATE A NEW ACCOUNT

        printf("\nYou have selected to create a checkings or savings account.\n");
          
        ACCOUNT_CREATION_FAILED:; // NO FUNCTION UNLESS GOTO
          char account_name[MAX_ACCOUNT_NAMES_LENGTH];
          int initial_balance;

          int checking_saving = checkings_savings_screen(
              user_info_index, account_name, &initial_balance);

          if (checking_saving == 1) {
            // create a checkings account
            add_account(database, 1, initial_balance, user_info_index, account_name);
            save_database(DATABASE_FILE);

            printf("\nCheckings account created successfully.\n");

            sleep(2);

            system("clear");

					goto ACCOUNT_SCREEN;
          } else if (checking_saving == 2) {
            // create savings account
            add_account(database, 2, initial_balance, user_info_index, account_name);
            save_database(DATABASE_FILE);

            printf("\nSavings account created successfully.\n");

            sleep(2);

            system("clear");

					goto ACCOUNT_SCREEN;
          } else if (checking_saving == 123) {
            goto ACCOUNT_SCREEN;
          }

          else {
            // account creation failed
            goto ACCOUNT_CREATION_FAILED;
          }
          
    } else if (menu_account == 2) {
           // TRANSFER
          transfer_money(user_info_index);
					goto ACCOUNT_SCREEN;
    } else if (menu_account == 3) { // DEPOSIT / WITHDRAW
      	int menu_option_WD, menu_option_D, menu_option_W;
      	double menu_option_amount;
        int i, j = 1;

        printf("\nSelect an option.\n\n1.) Withdraw \n2.) Deposit \n\n\033[32mEnter Selection: \033[0m");
        scanf("%d", &menu_option_WD);
					
      if (menu_option_WD == 1) {
        	printf("\nSelect which account to withdraw from\n");
				
			printf("\n\033[38;2;255;165;0m=== Accounts ===\n\n");
				for (int i = 0; i < database[user_info_index].num_accounts; i++) {
					printf("%d.) %s Account: %s Balance: %.2lf\n\n",
                 i + 1,
								 database[user_info_index].account_types[i],
								 database[user_info_index].account_names[i],
								 database[user_info_index].account_balances[i]);
				}
				printf("================\033[0m\n");
				

          
      		printf("\n\033[32mEnter selection: \033[0m");
    			scanf("%d", &menu_option_W);

					printf("\nAmount to withdraw? ");
        	scanf("%lf", &menu_option_amount);
				
					if (menu_option_W <= database[user_info_index].num_accounts 
							&& menu_option_amount <= database[user_info_index].account_balances[menu_option_W - 1])
					{	
						database[user_info_index].account_balances[menu_option_W - 1] -= menu_option_amount;
						save_database(DATABASE_FILE);

            printf("\nWithdraw successful.\n");
						
					} else {
       		 	printf("\nCannot withdraw %.2lf with account balance of %.2lf\n",menu_option_amount,database[user_info_index].account_balances[menu_option_W - 1]);
            sleep(3);
            goto ACCOUNT_SCREEN;
					}

      } else if (menu_option_WD == 2) {

      	printf("\nSelect which account to deposit into\n");
				
				printf("\n\033[38;2;255;165;0m=== Accounts ===\n\n");
				for (int i = 0; i < database[user_info_index].num_accounts; i++) {
					printf("%d.) %s Account: %s Balance: %.2lf\n\n",
                 i + 1,
								 database[user_info_index].account_types[i],
								 database[user_info_index].account_names[i],
								 database[user_info_index].account_balances[i]);
				}
				printf("================\033[0m\n");


        printf("\n\033[32mEnter selection: \033[0m");
				scanf("%d", &menu_option_D); 
						
            if (menu_option_D <= database[user_info_index].num_accounts) {
              printf("\nAmount to deposit? ");
              scanf("%lf", &menu_option_amount);
              database[user_info_index].account_balances[menu_option_D - 1] += menu_option_amount;

              printf("\nDeposit successful.\n");
              
              printf("\nNew balance is: %.2lf\n", database[user_info_index].account_balances[menu_option_D - 1]);
              save_database(DATABASE_FILE);
            }
          }

          else {
            printf("\nInvalid selection.\n");
          }

          sleep(3);

          system("clear");
          
			goto ACCOUNT_SCREEN;
					
  	} else if (menu_account == 4) { // cp interest calc
          
      printf("\nYou have selected interest calculator.\n");
          
    	int account_option;
      double ir_amount;
      double cmp_rate;
      printf("\nSelect which savings account to project (checking accounts N/A): \n");;
					
      printf("\n\033[38;2;255;165;0m=== Accounts ===\n\n");
				for (int i = 0; i < database[user_info_index].num_accounts; i++) {
					printf("%d.) %s Account: %s Balance: %.2lf\n\n",
                 i + 1,
								 database[user_info_index].account_types[i],
								 database[user_info_index].account_names[i],
								 database[user_info_index].account_balances[i]);
				}
				printf("================\033[0m\n");
					
 			printf("\n\033[32mEnter selection: \033[0m");
      scanf("%d", &account_option);
			if(account_option > database[user_info_index].num_accounts) {
				printf("\nInvalid account selection.\n");
        sleep(2);
				goto ACCOUNT_SCREEN;
			}
          
			if (strcmp(database[user_info_index].account_types[account_option - 1], "Checkings") == 0) {
				printf("\nInvalid account selection.\n");
				sleep(2);
				goto ACCOUNT_SCREEN;
			}
          
      int balance = database[user_info_index].account_balances[account_option - 1];
          
			if (balance < 25000) {
      	printf("\nNeed at least $25,000 for this feature.\n");
        sleep(3);
        system("clear");
				goto ACCOUNT_SCREEN;
      }
          
      printf("\nEnter interest rate (do not include percent sign): ");
      scanf("%lf", &ir_amount);
      printf("\nEnter compound interest frequency per year: ");
      scanf("%lf", &cmp_rate);

      if (balance >= 25000) {

      printf("\n5 Years: %.2lf\n10 Years: %.2lf\n15 Years: %.2lf\n20 Years: %.2lf\n", 
      ir_calc(balance, ir_amount, cmp_rate, 5), ir_calc(balance, ir_amount, cmp_rate, 10), 
      ir_calc(balance, ir_amount, cmp_rate, 15), ir_calc(balance, ir_amount, cmp_rate, 20));

        char key;
          printf("\nEnter any character to continue: ");
          key = getchar();
          if ( getchar() ) {
            goto ACCOUNT_SCREEN;
          }
      }
					
      goto ACCOUNT_SCREEN;
    }
  }
    } else if (menu_option == 123) {
      break;
    } else {
      // error incorrect value
      printf("\nInvalid menu option.\n");
      sleep(2);
      system("clear");
      main_menu();
    }
  }
  return 0;
} 