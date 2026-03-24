
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

typedef struct wallet { //create struct wallet to hold the data
    char category[35];
    int income;
    int expense;
    time_t date;
}wallet;

int wallet_reallocation(wallet **arr,int *wallet_capacity) {
    *wallet_capacity *= 2; //double the capacity
    wallet *tmp= realloc(*arr, *wallet_capacity * sizeof(*tmp));
    if (!tmp) return 0;
    *arr = tmp;
    printf("Wallet capacity increased to %d\n", *wallet_capacity);
    return 1;
}


void display_balance(wallet *walletarr, int walletcount)
{
    int totalIncome = 0;
    int totalExpense = 0;

    for (int i = 0; i < walletcount; i++) {
        if (walletarr[i].income > 0)
            totalIncome += walletarr[i].income;

        if (walletarr[i].expense > 0)
            totalExpense += walletarr[i].expense;
    }

    int balance = totalIncome - totalExpense;

    printf("\n=== Overall Wallet Summary ===\n");
    printf("Total Income : %d\n", totalIncome);
    printf("Total Expense: %d\n", totalExpense);
    printf("Balance      : %d\n", balance);
}


void most_expensive(char Category[][25], char lastupdate[][25], int CateNum,
    wallet *walletarr, int walletcount)
{
	int maxExp = -1; //maximum expense found
	int maxIndex = -1; //index of the most expensive category

    for (int i = 0; i < CateNum; i++) {
        int totalExp = 0;

        for (int j = 0; j < walletcount; j++) {
            if (strcmp(walletarr[j].category, Category[i]) == 0) {
                if (walletarr[j].expense > 0)
                    totalExp += walletarr[j].expense;
            }
        }

        if (totalExp > maxExp) {
			maxExp = totalExp; //update maximum expense
            maxIndex = i;
        }
    }

    if (maxIndex == -1) {
		printf("No expenses found!\n"); //if maxIndex is -1, no expenses were found
        return;
    }

	printf("\n=== Most Expensive Category ===\n"); //print the most expensive category
    printf("Category: %s\n", Category[maxIndex]);
    printf("Total Expense: %d\n", maxExp);
}

void list_categories_in_period(char Category[][25],
    char lastupdate[][25],
    int *CateNum,
    wallet *walletarr, int walletcount,
    time_t from, time_t to)
{
	if (from == -1 || to == -1) { // invalid date parse
        printf("Invalid date range\n");
        return;
    }
    /* array to mark which categories had activity */
    int seen[35] = { 0 };

    /* totals per category in period */
    int totInc[35] = { 0 };
    int totExp[35] = { 0 };

    for (int i = 0; i < walletcount; ++i) {
        time_t d = walletarr[i].date;
        if (d == 0) continue; /* skip entries without date */
		if (d < from || d > to) continue; // skip out-of-period entries

        /* find category index for this entry */
        for (int c = 0; c < *CateNum; ++c) {
            if (strcmp(walletarr[i].category, Category[c]) == 0) {
                seen[c] = 1;
				if (walletarr[i].income > 0) totInc[c] += walletarr[i].income; // accumulate income
				if (walletarr[i].expense > 0) totExp[c] += walletarr[i].expense; // accumulate expense
                break;
            }
        }
    }

    /* print result */
    printf("Categories with activity between the given dates:\n");
    int any = 0;
	int maxindex = -1; //maximum expense found
	int maxExp = -1; //maximum expense amount
    for (int c = 0; c < *CateNum; ++c) {
        if (seen[c]) {
            any = 1;
			char frombuf[32], tobuf[32]; // buffers for date strings
			strftime(frombuf, sizeof frombuf, "%Y-%m-%d", localtime(&from)); // format 'from' date
			strftime(tobuf, sizeof tobuf, "%Y-%m-%d", localtime(&to)); // format 'to' date
            printf("%d - %s\tIncome: %d\tExpense: %d\tLast update: %s\n",
                c + 1, Category[c], totInc[c], totExp[c],
                lastupdate[c][0] ? lastupdate[c] : "N/A");
            int totalex = 0;
            for (int j = 0; j < walletcount; j++) {
                if (strcmp(walletarr[j].category, Category[c]) == 0) {
                    if (walletarr[j].expense > 0)
                        totalex += walletarr[j].expense;
                }

            }
            if (totalex > maxExp) {
                maxExp = totalex; //update maximum expense
                maxindex = c;
            }
        }
        if (maxindex == -1) {
			printf("No expenses found!\n"); //if maxIndex is -1, no expenses were found
			return;
        }
    }
    printf("\n=== Most Expensive Category ===\n"); //print the most expensive category
    printf("Category: %s\n", Category[maxindex]);
    printf("Total Expense: %d\n", maxExp);

    if (!any) printf("No activity in the given period.\n");
}


/* parse "YYYY-MM-DD"  into time_t (local time).
   returns (time_t)-1 on parse failure */
time_t parse_datetime(const char* s) {
    if (s == NULL) return -1;
    int year = 0;
    int mon = 0;
    int day = 0;
    if (sscanf(s, "%d-%d-%d", &year, &mon, &day) < 3) return -1;

    /* basic range checks */
    if (year < 1900 || year > 9999) return -1; /* pick sensible bounds */
    if (mon < 1 || mon > 12) return -1;
    if (day < 1) return -1;

    /* days in month, account for leap year */
    int dim;
    switch (mon) {
    case 1: case 3: case 5: case 7: case 8: case 10: case 12:
        dim = 31; break;
    case 4: case 6: case 9: case 11:
        dim = 30; break;
    case 2: {
        int is_leap = ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
		if (is_leap) dim = 29; else dim = 28;
        break;
    }
    default:
        return -1;
    }
    if (day > dim) return -1;

    struct tm tm = { 0 };
	if (sscanf(s, "%d-%d-%d", &year, &mon, &day) >= 3) { // parse year, month, day
		tm.tm_year = year - 1900; // years since 1900
		tm.tm_mon = mon - 1; // months since January
		tm.tm_mday = day; // day of the month
		return mktime(&tm); // convert to time_t
    }
    return -1;
}

void set_now_timestamp(char lastupdate[25]) {
    time_t now = time(NULL);
	struct tm* t = localtime(&now); // convert to local time structure
    snprintf(lastupdate, 25, "%04d-%02d-%02d",
	t->tm_year + 1900, t->tm_mon + 1, t->tm_mday); // format as "YYYY-MM-DD"
}


void Add(wallet **wallet, int *walletcount,int cate, char (*Category)[25],char (*lastupdate)[25], int *wallet_capacity) {
    int indx = *walletcount;

    if (*walletcount >= *wallet_capacity) {
        if (!wallet_reallocation(wallet, wallet_capacity)) {
            printf("Failed to grow wallet array\n");
            return;
        }
    }

    if (cate <= 0 || cate > 20) {
        printf("Invalid category index\n");
        return;
    }


    strcpy((*wallet)[indx].category, Category[cate - 1]);

    printf("Enter the income:\n");
    if(scanf("%d", &(*wallet)[indx].income) != 1) {
        system("CLS");
        printf("Invalid input for income!\n");
        while (getchar() != '\n');
        return;
	}
    if ((*wallet)[indx].income < 0) {
        printf("Income cannot be negative!\n");
        return;   // STOP and return to manage
    }

    printf("Enter the expense:\n");
    if (scanf("%d", &(*wallet)[indx].expense) != 1) {
        system("CLS");
        printf("Invalid input for expense!\n");
        while (getchar() != '\n');
        return;
    }
    if ((*wallet)[indx].expense < 0) {
        printf("Expense cannot be negative!\n");
        return;   // STOP and return to manage
    }
    set_now_timestamp(lastupdate[cate - 1]);
    (*wallet)[indx].date = time(NULL);
    (*walletcount)++;

    printf("Record was added successfully!\n");
}




void load(int *walletcount, wallet **wallet, char Category[][25], int *CateNum,char lastupdate[][25], int capacity, int *wallet_capacity) {
    FILE* file = fopen("wallet.txt", "r");
    if (file == NULL) {
        printf("Error, no data in the file\n");
        *walletcount = 0;
        return;

    }


    int CateNum_local = 0;
    int saved_wallet = 0;

    /* read CateNum and saved walletcount */
    if (fscanf(file, "%d %d\n", &CateNum_local, &saved_wallet) != 2) {
        printf("File format error (missing header)\n");
        fclose(file);
        *walletcount = 0;
        return;
    }
	*CateNum = CateNum_local;

    for (int i = 0; i < CateNum_local && i < capacity; ++i) {

        if (fgets(Category[i], 25, file) == NULL) {
            Category[i][0] = '\0';
        }
        else {
            Category[i][strcspn(Category[i], "\n")] = '\0';
        }


        if (fgets(lastupdate[i], 25, file) == NULL) {
            lastupdate[i][0] = '\0';      /*  make empty string */
        }
        else {
            lastupdate[i][strcspn(lastupdate[i], "\n")] = '\0';  /* strip newline */
        }
    }



    char line[256];
    *walletcount = 0;
    while (*walletcount < *wallet_capacity && fgets(line, sizeof(line), file) != NULL) {
        char catbuf[35];
        int inc = -1, exp = -1;
		long long epoch = 0;
        int scanned = sscanf(line, " %34[^|]|%d|%d|%lld", catbuf, &inc, &exp,&epoch);
        if (scanned >= 1) {
            if (*walletcount >= *wallet_capacity) {
                if (!wallet_reallocation(wallet, wallet_capacity)) {
                    printf("Failed to grow wallet array while loading\n");
                    break;
                }
            }
            /* copy category into wallet entry */
            strncpy((*wallet)[*walletcount].category, catbuf, sizeof((*wallet)[*walletcount].category) - 1);
            (*wallet)[*walletcount].category[sizeof((*wallet)[*walletcount].category) - 1] = '\0';

            if (scanned >= 3) {
                (*wallet)[*walletcount].income = inc;
                (*wallet)[*walletcount].expense = exp;
            }
            else {
                (*wallet)[*walletcount].income = -1;
                (*wallet)[*walletcount].expense = -1;
            }
            if (scanned == 4) (*wallet)[*walletcount].date = (time_t)epoch;
            else (*wallet)[*walletcount].date = 0; /* or some sentinel */
            (*walletcount)++;
        }


    }

    fclose(file);
    printf("Data loaded! %d entries, %d categories\n", *walletcount, CateNum_local);

        int load;
        printf("1-Dicplay data\n2-Display the most expensive data\n3-Display peroidic categories\n4-Display Balance");
        scanf("%d", &load);
        if (load == 1) {

            printf("Categories in the wallet:\n");
            for (int i = 0; i < *CateNum; i++) {
                int inc = 0, exp = 0;

                for (int j = 0; j < *walletcount; j++) {
					if (strcmp((*wallet)[j].category, Category[i]) == 0) { // add the entries if similar category found
                        if ((*wallet)[j].income > 0) inc += (*wallet)[j].income;
                        if ((*wallet)[j].expense > 0) exp += (*wallet)[j].expense;
                    }
                }

                printf("%d- %s\tIncome: %d\tExpense: %d\tLast update: %s\n",
                    i + 1, Category[i], inc, exp, lastupdate[i][0] ? lastupdate[i] : "N/A");
            }



        }
        if (load == 2) {
            most_expensive(Category, lastupdate, *CateNum, *wallet, *walletcount);
        }
        if(load==3){
            char frombuf[32], tobuf[32];
            printf("Enter the 'from' date (YYYY-MM-DD): ");
            scanf("%31s", frombuf);
            printf("Enter the 'to' date (YYYY-MM-DD): ");
            scanf("%31s", tobuf);
            time_t from = parse_datetime(frombuf);
            time_t to = parse_datetime(tobuf);
            list_categories_in_period(Category, lastupdate, CateNum, *wallet, *walletcount, from, to);
		}
		if (load == 4) {
            display_balance(*wallet, *walletcount);
        }




    }


void manage(wallet **wallet,int *walletcount, int *wallet_capacity) {
    char (*Category)[25] = NULL;              // dynamic 2D array
    char (*lastupdate)[25] = NULL;              // dynamic 2D array
        int CateNum = 0;
        int capacity = 3;
        int cate;
        //memory allocation
        Category = calloc(capacity , sizeof * Category);
		lastupdate = calloc(capacity , sizeof * lastupdate);
        if (Category == NULL || lastupdate == NULL) {
            printf("Memory allocation failed\n");
            goto cleanup;
            return;
        }

        // add initial categories
        strncpy(Category[0], "Household", 24);
        Category[0][24] = '\0';
        strncpy(Category[1], "Salary", 24);
        Category[1][24] = '\0';
        set_now_timestamp(lastupdate[0]);
        set_now_timestamp(lastupdate[1]);
        CateNum = 2;


        while (1) {
            printf("Choose a category to enter income/expense:\n");
            for (int i = 0; i < CateNum; i++) {
                printf("%d - %s\n", i + 1, Category[i]);
            }
                printf("Other:\n%d - Settings\n%d - Add new category\n", CateNum+1, CateNum+2);
                printf("Or enter 0 to exit");

                if (scanf("%d", &cate) != 1) {
                    system("CLS"); //additioanal function to clear screen
					printf("Invalid input!\n");
                    while (getchar() != '\n');
					continue;
                }
            if (cate < 0 || cate>CateNum+2) {
                system("CLS"); //additioanal function to clear screen
                printf("Invalid choice!\n");
				continue;
            }

            if (cate == 0) {
                break;
            }
            if (cate == CateNum + 2) {
                char newcate[22];
                printf("Insert the new category:\n");
                /* flush leftover newline from previous scanf */
                int ch;
                while ((ch = getchar()) != '\n' && ch != EOF);

                /* read the new category safely */
                if (fgets(newcate, sizeof newcate, stdin) == NULL) newcate[0] = '\0';
                else newcate[strcspn(newcate, "\n")] = '\0';  /* strip trailing newline */

                //increasing the capacity if needed by multiplying by 2
                if (CateNum == capacity) {
                    capacity *= 2;
                    char (*tmpcate)[25] = realloc(Category, capacity * sizeof * Category); // reallocating memory to the new size
                    char (*tmptime)[25] = realloc(lastupdate, capacity * sizeof * lastupdate); // reallocating memory to the new size
                    if (tmpcate == NULL || tmptime == NULL) {
                        printf("Memory reallocation failed\n");
                        goto cleanup;
                        return;
                    }
                    Category = tmpcate;
                    lastupdate = tmptime;
                }


                strcpy(Category[CateNum], newcate);

                set_now_timestamp(lastupdate[CateNum]);

                CateNum++;
                printf("Category was added successfully\n");
				printf("Please save the category in settings menu\n");
            }

             if (cate == CateNum+1) {
                while (1) {
                    int set ;
                    printf("1-Save category\n2-Load category\n3-Exit to category menu");
                    if (scanf("%d", &set) != 1) {
                        system("CLS"); //additioanal function to clear screen
                        printf("Invalid input\n");
                        while (getchar() != '\n');
                        continue;

                    }
                    if (set < 1 || set >3) {
                        system("CLS"); //additioanal function to clear screen
                        printf("Invalid choise\n");
                        continue;
                    }
                    if (set == 3) {
                        break;
                    }


                    if (set == 1) {
                    FILE* file = fopen("wallet.txt", "w");
                        if (file == NULL) {
                        printf("Error could not open the file\n");
                        break;
                    }

                        /* save categories */
                        printf("DEBUG: About to save. CateNum=%d, walletcount=%d\n", CateNum, *walletcount);
                        for (int i = 0; i < *walletcount; ++i) {
                            printf("DEBUG: entry %d => category=\"%s\" income=%d expense=%d\n",
                                i, (*wallet)[i].category, (*wallet)[i].income, (*wallet)[i].expense);
                        }

                        fprintf(file, "%d %d\n", CateNum, *walletcount);
                        for (int i = 0; i < CateNum; ++i) {
                            fprintf(file, "%s\n", Category[i]);
                            fprintf(file, "%s\n", lastupdate[i]);
                        }

                        /* save entries as categoryName|income|expense */
                        for (int i = 0; i < *walletcount; ++i) {
                            /* removing '|' out of category name */
                            for (char* p = (*wallet)[i].category; *p; ++p) if (*p == '|') *p = '_';
                            fprintf(file, "%s|%d|%d|%lld\n",
                                (*wallet)[i].category,
                                (*wallet)[i].income,
                                (*wallet)[i].expense,
                                (long long) (*wallet)[i].date);
                        }



                        fclose(file);
                        printf("Wallet was saved successfully\n");
                    }
                    else if (set == 2) {
                        load(walletcount , wallet,Category, &CateNum,lastupdate,capacity, wallet_capacity);
                    }

                }
            }
             else {
                 Add(wallet, walletcount, cate, Category,lastupdate, wallet_capacity);
             }
        }

    cleanup:
	free(Category);
	free(lastupdate);
    return;
}

int main() {
    wallet *wallet = NULL  ;
	int wallet_capacity = 50;
    int walletcount = 0;
    int choice;

    wallet = malloc(wallet_capacity * sizeof(wallet));
    if (!wallet) {
        printf("Failed to allocate the wallet\n");
        return 1;
    }

    while (1){
        printf("Choose a choice please:\n1-MANAGE WALLET\n2-EXIT\n");
		if (scanf("%d", &choice) != 1) {
            system("CLS"); //additioanal function to clear screen
            printf("Invalid input!\n");
            while (getchar() != '\n');
            continue;
        }
        if (choice > 2 || choice < 1) {
            system("CLS"); //additioanal function to clear screen
            printf("Invalid choice!\n");
        }
        if (choice == 1) {

            manage(&wallet,&walletcount, &wallet_capacity);
        }
        if (choice == 2) {
            break;
        }

	}

    free(wallet);
    return 0;
}
