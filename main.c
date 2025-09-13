#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "encryption.c"

#define MAX_ATTEMPTS 3
#define LOCK_TIME 10000
#define PASSWORD_LENGTH 8

#define AES_KEY_SIZE 32 // 32 Byte: 256-bit key
#define AES_IV_SIZE 16  // Vector Initialization IV: size for AES-CBC 16 Byte = 128-bit

// 30 + 16 + 8 + 8 = 62 Byte
unsigned char plaintext[256], ciphertext[256], decryptedtext[256];
unsigned char key[AES_KEY_SIZE], iv[AES_IV_SIZE];

// any function in OpenSSL return 1-Success, [0 or -1] Fail
// EVP: Encrypt or Decrypt  error > call handleErors()

float amount = 0.00, depositAmount, withdrawAmount, transferAmount;
int found, attempts = 0;
long long accountID;
char username[30], password[PASSWORD_LENGTH + 1], dataUsername[50], dataPassword[PASSWORD_LENGTH + 1], confirmProcessPassword[PASSWORD_LENGTH + 1], transferUsername[50];

char adminUserAccount[] = "admin", adminUserPassword[] = "Eror404S";

void createAccount();
void login();
void adminPanel();

void accountDetails();
void depositMoney();
void transferMoney();
void withdrawMoney();
void payment();
void settings();
void handleErrors();


void handleErrors()
{
    printf("Error occurred!\n");
    exit(1);
}

int main()
{
    srand(time(0)); // Seed for random account number generation

    FILE *encFile = fopen("buffer.txt", "rb");   // read Encrypted File
    FILE *decFile = fopen("data.txt", "w");    // Dealing
    if (!encFile || !decFile)
    {
        perror("Error opening files!");
        return 1;
    }

    //Read IV
    fread(iv, 1, AES_IV_SIZE, encFile);

    int ciphertext_len, decryptedtext_len;
    while (fread(&ciphertext_len, sizeof(int), 1, encFile))
    {
        fread(ciphertext, 1, ciphertext_len, encFile);
        decryptAES(ciphertext, ciphertext_len, key, iv, decryptedtext, &decryptedtext_len);
        decryptedtext[decryptedtext_len] = '\0';
        fprintf(decFile, "%s", decryptedtext);
    }

    fclose(encFile);
    fclose(decFile);

    int initialChoice;
    printf("Welcome to the Errors 404 Banking System\n");
    printf("1. Create a new account\n");
    printf("2. Login to existing account\n");
    printf("Enter your choice: ");
    scanf("%d", &initialChoice);

    switch (initialChoice)
    {
    case 1:
        createAccount();
        break;
    case 2:
        login();
        break;
    default:
        printf("Invalid choice. Exiting...\n");
        exit(0);
    }

    while (1)
    {
        printf("\nMain Menu\n");
        printf("1 - Account details\n");
        printf("2 - Deposit\n");
        printf("3 - Withdraw\n");
        printf("4 - Transfer\n");
        printf("5 - Payment\n");
        printf("6 - Settings\n");
        printf("7 - Exit\n");

        int choice;
        printf("Enter your choice: ");
        scanf("%d", &choice);
        while (getchar() != '\n');

        switch (choice)
        {
        case 1:
            system("cls");
            printf("\nDetails is Selected.\n");
            accountDetails();
            break;
        case 2:
            system("cls");
            printf("\nDeposit is Selected.\n");
            depositMoney();
            break;
        case 3:
            system("cls");
            printf("\nWithdraw is Selected.\n");
            withdrawMoney();
            break;
        case 4:
            system("cls");
            printf("\nTransfer is Selected.\n");
            transferMoney();
            break;
        case 5:
            system("cls");
            printf("\nPayment is Selected.\n");
            payment();
            break;
        case 6:
            system("cls");
            printf("\nSettings is Selected.\n");
            settings();
            break;
        case 7:
            system("cls");
            printf("\nWe were happy serving you.\n");
            // Re-encrypt
            FILE *normalFile = fopen("data.txt", "r");
            FILE *encFileSave = fopen("buffer.txt", "wb");
            if (!normalFile || !encFileSave)
            {
                perror("Error opening files for encryption!");
                return 1;
            }

            fwrite(iv, 1, AES_IV_SIZE, encFileSave); // Save IV

            while (fgets((char *)plaintext, sizeof(plaintext), normalFile))
            {
                encryptAES(plaintext, strlen((char *)plaintext), key, iv, ciphertext, &ciphertext_len);
                fwrite(&ciphertext_len, sizeof(int), 1, encFileSave);
                fwrite(ciphertext, 1, ciphertext_len, encFileSave);
            }

            fclose(normalFile);
            fclose(encFileSave);
            remove("data.txt");
            exit(1);
        default:
            printf("Invalid choice\n");
        }
    }
    return 0;
}

void createAccount()
{
    FILE* fptr,* logptr;
    fptr = fopen("data.txt","a+");

    printf("\nCreate your account\n");
    do
    {
        found = 0;

        printf("Enter your username: ");
        scanf("%s", username);
        do
        {
            if (strlen(username) > 30)
            {
                printf("Username cannot exceed 30 characters. Please try again.\n");
                printf("Enter your username: ");
                scanf("%s", username);
            }
        }
        while (strlen(username) > 30);

        rewind(fptr);

        while (fscanf(fptr, "%s   %d     %s     %f",&dataUsername, &accountID, &dataPassword, &amount) == 4)
        {
            if (strcmp(dataUsername, username) == 0)
            {
                found = 1;
                printf("This username is already taken. Please choose another username.\n");
                break;
            }
        }

    }
    while (found);

    printf("Password must be 8 characters [numbers, symbols and letters].\n");
    passwordChecker();

    accountID = 100000000000LL + ((long long)rand() * rand()) % 900000000000LL;

    logptr = fopen("log\\create.txt", "a");
    fprintf(logptr, "%s - New account created - Username: %s, Account No: [%lld]\n",currentTime(), username, accountID);
    fclose(logptr);

    printf("\nYour account has been created successfully!\n\n");
    printf("\n# # ####    DONT FORGET YOUR ACCOUNT NUMBER SIR   #### # #\n");
    printf("# #  YOUR ACCOUNT NUMBER: %lld  # #\n", accountID);
    printf("# #  YOUR ACCOUNT USERNAME: %s  # #\n", username);
    printf("# #  YOUR ACCOUNT PASSWORD: %s  # #\n", password);

    fprintf(fptr, "%s   %lld     %s     %.2f\n", username, accountID, password, amount);
    fclose(fptr);

}

void login()
{
    FILE* fptr,* logptr;
    int exist = 0;

    char amr;

    printf("\nLogin to your account\n");
    while(attempts < MAX_ATTEMPTS)
    {
        fptr = fopen("data.txt","r");
        logptr = fopen("log//login.txt", "a");
        printf("Enter your account username: ");
        scanf("%s", username);

        printf("Enter your account password: ");
        scanf("%s", password);
        while (getchar() != '\n');

        if (strcmp(username, adminUserAccount) == 0 && (strcmp(password, adminUserPassword) == 0))
        {
            printf("\nAdmin login successful! Welcome, Administrator.\n");
            adminPanel();
            return;
        }

        while (fscanf(fptr, "%s   %lld     %s     %f",&dataUsername, &accountID, &dataPassword, &amount) == 4)
        {
            if (strcmp(dataUsername, username) == 0 && (strcmp(dataPassword, password) == 0))
            {
                exist = 1;
                break;
            }
        }
        fclose(fptr);

        if (exist)
        {
            printf("Login successful! Welcome back.\n");
            fprintf(logptr, "%s - LOGIN SUCCESS - Username: %s\n", currentTime(), username);
            fclose(logptr);
            return;
        }
        else
        {
            attempts++;
            printf("Error: Invalid username or password! Attempts left: %d\n", MAX_ATTEMPTS - attempts);
            fprintf(logptr, "%s - LOGIN FAILED - Username: %s\n", currentTime(), username);

        }
        fclose(logptr);
    }
    printf("\nToo many failed attempts. Access denied!\n");
    printf("Secion ending please wait 15 sec ");
    Sleep(LOCK_TIME);
    exit(2);
}

void accountDetails()
{
    FILE* fptr;
    fptr = fopen("data.txt","r");
    rewind(fptr);
    while (fscanf(fptr, "%s   %lld     %s     %f",&dataUsername, &accountID, &dataPassword, &amount) == 4)
    {
        if (strcmp(dataUsername, username) == 0 && (strcmp(dataPassword, password) == 0))
        {
            printf("Your account number: %lld\n", accountID);
            printf("Your username is: %s\n", dataUsername);
            printf("Your account password: %s\n", dataPassword);
            printf("Your current balance: %.2f LE\n", amount);
            break;
        }
    }

    fclose(fptr);
}

void depositMoney()
{
    found = 0;
    FILE* fptr,* temp,* logptr;

    printf("Enter the amount: ");
    scanf("%f", &depositAmount);
    while (getchar() != '\n');
    if (depositAmount <= 0)
    {
        printf("Invalid amount! Please enter a valid number.\n");
        return;
    }
    fptr = fopen("data.txt","r");
    temp = fopen("temp.txt","w");
    logptr = fopen("log\\deposit.txt","a");

    printf("Enter your account password: ");
    scanf("%s", confirmProcessPassword);
    while (getchar() != '\n');

    while (fscanf(fptr, "%s   %lld     %s     %f",&dataUsername, &accountID, &dataPassword, &amount) == 4)
    {

        if (strcmp(dataUsername, username) == 0 && (strcmp(dataPassword, confirmProcessPassword) == 0))
        {
            amount += depositAmount;
            printf("Money deposited successfully.\n");
            printf("Your current balance: %.2f LE\n", amount);
            found = 1;
            fprintf(logptr, "%s - DEPOSIT SUCCESS - Username: %s, Amount: %.2f LE, New Balance: %.2f LE\n",currentTime(), username, depositAmount, amount);
        }
        fprintf(temp, "%s   %lld     %s     %.2f\n", dataUsername, accountID, dataPassword, amount);

    }
    fclose(fptr);
    fclose(temp);
    fclose(logptr);

    if (!found)
    {
        printf("The password you entered is incorrect.\n");
        remove("temp.txt");
        return;
    }
    remove("data.txt");
    rename("temp.txt", "data.txt");
}

void withdrawMoney()
{
    FILE* fptr,* temp,* logptr;
    found = 0;

    printf("Enter the amount to withdraw: ");
    scanf("%f", &withdrawAmount);
    while (getchar() != '\n');
    if (withdrawAmount > amount)
    {
        printf("Insufficient balance.\n");
        return;
    }
    else if(withdrawAmount <= 0)
    {
        printf("Insufficient balance.\n");
        return;
    }
    fptr = fopen("data.txt","r");
    temp = fopen("temp.txt","w");
    logptr = fopen("log\\withdraw.txt","a");

    printf("Enter your account password: ");
    scanf("%s", confirmProcessPassword);

    while (fscanf(fptr, "%s   %lld     %s     %f",&dataUsername, &accountID, &dataPassword, &amount) == 4)
    {
        if (strcmp(dataUsername, username) == 0 && (strcmp(dataPassword, confirmProcessPassword) == 0))
        {
            amount -= withdrawAmount;
            printf("Money withdrawn\n");
            printf("Your current balance: %.2f LE\n", amount);
            found = 1;
            fprintf(logptr, "%s - WITHDRAW SUCCESS - Username: %s, Amount: %.2f LE, New Balance: %.2f LE\n",currentTime(), username, withdrawAmount, amount);
        }
        fprintf(temp, "%s   %lld     %s     %.2f\n", dataUsername, accountID, dataPassword, amount);
    }
    fclose(logptr);
    fclose(fptr);
    fclose(temp);

    if (!found)
    {
        printf("The password you entered is incorrect.\n");
        remove("temp.txt");
        return;
    }

    remove("data.txt");
    rename("temp.txt", "data.txt");
}

void transferMoney()
{
    int recipient = 0, found = 0;

    printf("Enter the amount to transfer: ");
    scanf("%f", &transferAmount);
    while (getchar() != '\n');
    if(transferAmount > amount)
    {
        printf("You don't have enough money!\n");
        return;
    }
    else if (transferAmount <= 0)
    {
        printf("Invalid amount! Please enter a valid number.\n");
        return;
    }

    FILE* fptr,* temp,* logptr;
    fptr = fopen("data.txt","r");
    temp = fopen("temp.txt","w");
    logptr = fopen("log\\transactions.txt","a");

    printf("Enter the account to transfer to: ");
    scanf("%s", transferUsername);

    while (fscanf(fptr, "%s   %lld     %s     %f", dataUsername, &accountID, &dataPassword, &amount) == 4)
    {
        if (strcmp(dataUsername, transferUsername) == 0)
        {
            recipient = 1;
        }
    }

    printf("Enter your account password to confirm: ");
    scanf("%s", confirmProcessPassword);

    if ((strcmp(dataPassword, confirmProcessPassword) == 0))
    {
        printf("Incorrect password. Transaction cancelled.\n");
        fclose(fptr);
        fclose(temp);
        remove("temp.txt");
        return;
    }

    if (!recipient)
    {
        printf("The recipient's account does not exist.\n");
        remove("temp.txt");
        return;
    }

    rewind(fptr);
    while (fscanf(fptr, "%s   %lld     %s     %f", dataUsername, &accountID, &dataPassword, &amount) == 4)
    {
        if (strcmp(dataUsername, username) == 0 && (strcmp(dataPassword, password) == 0))
        {
            if (strcmp(dataUsername, transferUsername) == 0)
            {
                printf("You can't transfer money to your own account.\n");
                fclose(fptr);
                fclose(temp);
                remove("temp.txt");
                return;
            }

            amount -= transferAmount;
            printf("Money transferred successfully.\n");
            printf("Your current balance: %.2f LE\n", amount);
            fprintf(logptr, "%s - TRANSFER SUCCESS - From Username: %s, to Username: %s Amount: %.2f LE, New Balance: %.2f LE\n",currentTime(), username, transferUsername, transferAmount, amount);
        }
        else if (strcmp(dataUsername, transferUsername) == 0)
        {
            amount += transferAmount;
        }
        fprintf(temp, "%s   %lld     %s     %.2f\n", dataUsername, accountID, dataPassword, amount);
    }
    fclose(logptr);
    fclose(fptr);
    fclose(temp);
    remove("data.txt");
    rename("temp.txt", "data.txt");
}

void payment()
{
    int paymentChoice, found = 0;
    printf("Payment Options:\n");
    printf("1. Internet = 240 LE\n2. Electricity = 2000 LE\n3. Gas = 110 LE\n4.Exit\n");
    printf("Choose which payment: ");
    scanf("%d", &paymentChoice);
    while (getchar() != '\n');

    int paymentAmount = 0;
    switch (paymentChoice)
    {
    case 1:
        paymentAmount = 240;
        break;
    case 2:
        paymentAmount = 2000;
        break;
    case 3:
        paymentAmount = 110;
        break;
    case 4:
        return 1;
    default:
        printf("Invalid payment choice\n");
        return;
    }
    if(paymentAmount > amount)
    {
        printf("Insufficient balance\n");
        return;
    }
    printf("Enter your account password: ");
    scanf("%s", confirmProcessPassword);

    FILE* fptr,* temp,* logptr;
    fptr = fopen("data.txt","r");
    temp = fopen("temp.txt","w");
    logptr = fopen("log\\payment.txt","a");

    while (fscanf(fptr, "%s   %lld     %s     %f",&dataUsername, &accountID, &dataPassword, &amount) == 4)
    {
        if (strcmp(dataUsername, username) == 0 && (strcmp(dataPassword, confirmProcessPassword) == 0))
        {
            amount -= paymentAmount;
            printf("Payment successful\n");
            printf("Your current balance: %.2f LE\n", amount);
            found = 1;
            fprintf(logptr, "%s - PAYMENT SUCCESS - Username: %s, Amount: %d LE, New Balance: %.2f LE\n",currentTime(), username, paymentAmount, amount);
        }
        fprintf(temp, "%s   %lld     %s     %.2f\n", dataUsername, accountID, dataPassword, amount);
    }
    fclose(logptr);
    fclose(fptr);
    fclose(temp);

    if (!found)
    {
        printf("The password you entered is incorrect.\n");
        remove("temp.txt");
        return;
    }
    remove("data.txt");
    rename("temp.txt", "data.txt");
}

void settings()
{
    FILE* fptr,* temp;
    fptr = fopen("data.txt","r");
    temp = fopen("temp.txt","w");
    int choice;
    char changeUsername[50];
    printf("Please select the settings you want to modify: \n1- Change your username.\n2- Change your password.\n");
    printf("Your choice: ");
    scanf("%d",&choice);
    switch(choice)
    {
    case 1:
        do
        {
            found = 0;

            printf("Enter your username: ");
            scanf("%s", changeUsername);

            rewind(fptr);
            while (fscanf(fptr, "%s   %d     %s     %f",&dataUsername, &accountID, &dataPassword, &amount) == 4)
            {
                if (strcmp(dataUsername, changeUsername) == 0)
                {
                    found = 1;
                    printf("This username is already taken. Please choose another username.\n");
                    break;
                }
                if (strcmp(dataUsername, username) == 0 && (strcmp(dataPassword, password) == 0))
                {
                    strcpy(dataUsername, changeUsername);
                    strcpy(username, changeUsername);
                    printf("\nYour Username changed Successfully!\n");
                }
                fprintf(temp, "%s   %lld     %s     %.2f\n", dataUsername, accountID, dataPassword, amount);
            }

        }
        while(found == 1);
        break;
    case 2:
        printf("Your New password must be 8 characters [numbers, symbols and letters].\n");

        while (fscanf(fptr, "%s   %lld     %s     %f",&dataUsername, &accountID, &dataPassword, &amount) == 4)
        {
            if (strcmp(dataUsername, username) == 0 && (strcmp(dataPassword, password) == 0))
            {
                passwordChecker();
                strcpy(dataPassword, password);
                printf("\nYour Password changed Successfully!\n");

            }
            fprintf(temp, "%s   %lld     %s     %.2f\n", dataUsername, accountID, dataPassword, amount);
        }
        break;

    default:
        printf("Please Enter Valid choice.");
    }

    fclose(fptr);
    fclose(temp);
    remove("data.txt");
    rename("temp.txt", "data.txt");
}

void adminPanel()
{
    int adminChoiceMain, adminChoiceSub;
    char log[512], searchUser[50];
    FILE* logptr;
    do
    {
        printf("\n==== Admin Panel ====\n");
        printf("1. View all logs\n");
        printf("2. Search logs by username\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &adminChoiceMain);

        switch(adminChoiceMain)
        {
        case 1:
            do
            {
                printf("\n==== Logs Panel ====\n");
                printf("1. View all Created Accounts.\n");
                printf("2. View all Login process.\n");
                printf("3. View all Deposits\n");
                printf("4. View all Withdraws.\n");
                printf("5. View all Transactions.\n");
                printf("6. View all Payments.\n");
                printf("7. Exit\n");
                printf("Enter your choice: ");
                scanf("%d", &adminChoiceSub);

                switch(adminChoiceSub)
                {
                case 1:
                    logptr = fopen("log\\create.txt", "r");
                    if (!logptr)
                    {
                        printf("No logs found!\n");
                        return;
                    }

                    printf("\n===== Accounts LOGS =====\n");
                    while (fgets(log, sizeof(log), logptr))
                    {
                        printf("%s", log);
                    }
                    fclose(logptr);
                    break;
                case 2:
                    logptr = fopen("log\\login.txt", "r");
                    if (!logptr)
                    {
                        printf("No logs found!\n");
                        return;
                    }

                    printf("\n===== Login LOGS =====\n");
                    while (fgets(log, sizeof(log), logptr))
                    {
                        printf("%s", log);
                    }
                    fclose(logptr);
                    break;
                case 3:
                    logptr = fopen("log\\deposit.txt", "r");
                    if (!logptr)
                    {
                        printf("No logs found!\n");
                        return;
                    }

                    printf("\n===== Deposit LOGS =====\n");
                    while (fgets(log, sizeof(log), logptr))
                    {
                        printf("%s", log);
                    }
                    fclose(logptr);
                    break;
                case 4:
                    logptr = fopen("log\\withdraw.txt", "r");
                    if (!logptr)
                    {
                        printf("No logs found!\n");
                        return;
                    }

                    printf("\n===== Withdraw LOGS =====\n");
                    while (fgets(log, sizeof(log), logptr))
                    {
                        printf("%s", log);
                    }
                    fclose(logptr);
                    break;
                case 5:
                    logptr = fopen("log\\transactions.txt", "r");
                    if (!logptr)
                    {
                        printf("No logs found!\n");
                        return;
                    }

                    printf("\n===== Transactions LOGS =====\n");
                    while (fgets(log, sizeof(log), logptr))
                    {
                        printf("%s", log);
                    }
                    fclose(logptr);
                    break;
                case 6:
                    logptr = fopen("log\\payment.txt", "r");
                    if (!logptr)
                    {
                        printf("No logs found!\n");
                        return;
                    }

                    printf("\n===== Payments LOGS =====\n");
                    while (fgets(log, sizeof(log), logptr))
                    {
                        printf("%s", log);
                    }
                    fclose(logptr);
                    break;
                case 7:
                    break;
                default:
                    printf("Invalid choice! Try again.\n");
                }
            }
            while(adminChoiceSub != 7);
            break;
        case 2:

            printf("Enter username to search: ");
            scanf("%s", searchUser);

            logptr = fopen("data.txt", "r");
            if (!logptr)
            {
                printf("No logs found!\n");
                return;
            }

            printf("\n===== SEARCH RESULTS FOR %s =====\n", searchUser);
            int userFound = 0;
            while (fgets(log, sizeof(log), logptr))
            {
                if (strstr(log, searchUser))
                {
                    printf("%s", log);
                    userFound = 1;
                }
            }
            if (!userFound)
            {
                printf("There is no account with this user..\n");
            }

            fclose(logptr);
            break;
        case 3:
            printf("Exiting admin panel...\n");
            exit(3);
        default:
            printf("Invalid choice! Try again.\n");
        }
    }
    while (1);
}
