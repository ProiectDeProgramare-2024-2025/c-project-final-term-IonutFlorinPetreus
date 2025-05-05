#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifdef __linux__
#define CLEAR_CONSOLE "clear"
#else
#define CLEAR_CONSOLE "cls"
#endif

#define CENT 1
#define ACC_ATRIBUTES_COUNT 5

typedef struct Account Account;

struct Account
{
    unsigned short id;
    char first_name[30];
    char last_name[30];
    char iban[30];
    int balance;
};

DIR *open_account_dir()
{
    DIR *dir = opendir("./accounts");
    if (dir == NULL)
    {
        printf("[\033[31;1mBank Error\033[0m] Nu exista un fisier pentru conturi (./accounts). Te rugam creeaza unul.\n");
        return NULL;
    }

    readdir(dir);
    readdir(dir);

    return dir;
}

void print_account(struct Account a)
{
    print_account(a);
}

void generate_random_string(char *str, size_t length)
{
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    size_t charset_size = sizeof(charset) - 1;

    if (length < 1)
    {
        str[0] = '\0';
        return;
    }

    for (size_t i = 0; i < length; i++)
    {
        str[i] = charset[rand() % charset_size];
    }
    str[length] = '\0';
}

int get_last_account_id()
{
    FILE *f = fopen("./accounts/_last_inserted_id", "r");
    int id = 0;
    if (f != NULL)
    {
        char s[10];
        fgets(s, 10, f);
        id = atoi(s);
        fclose(f);
    }
    return id;
}

void update_last_account_id()
{
    int id = get_last_account_id() + 1;
    FILE *f = fopen("./accounts/_last_inserted_id", "w");
    fprintf(f, "%d", id);
    fclose(f);
}

void write_account(struct Account *a)
{
    char file_name[70];
    sprintf(file_name, "./accounts/%d.acc", a->id);
    FILE *f = fopen(file_name, "w");
    fprintf(f, "%d;%s;%s;%s;%d\n", a->id, a->first_name, a->last_name, a->iban, a->balance);
    fclose(f);
    update_last_account_id();
}

void parse_account(struct Account *a, char *s, FILE *f)
{
    fgets(s, 125, f);
    s[strlen(s) - 1] = '\0';
    char t[ACC_ATRIBUTES_COUNT][70];
    __uint8_t h = 0;
    char *p = strtok(s, ";");
    while (p != NULL)
    {
        strcpy(t[h++], p);
        p = strtok(NULL, ";");
    }

    a->id = atoi(t[0]);
    strcpy(a->first_name, t[1]);
    strcpy(a->last_name, t[2]);
    strcpy(a->iban, t[3]);
    a->balance = atoi(t[4]);
}

int list_accounts(int pages)
{
    struct dirent *f;
    DIR *dir = open_account_dir();
    if (dir == NULL)
        return -1;

    char s[100];
    struct Account acc;

    int a = 10 * (pages - 1) + 1;
    int b = 10 * pages;
    int i = 0;
    while ((f = readdir(dir)) != NULL)
    {
        if (f->d_name[0] == '_')
            continue;
        i++;
        if (i < a)
            continue;

        if (i > b)
            break;

        FILE *db;
        sprintf(s, "./accounts/%s", f->d_name);
        db = fopen(s, "r");

        parse_account(&acc, s, db);
        print_account(acc);

        fclose(db);
    }
    return 0;
}

void _add_account_money(struct Account a, double amount)
{
    if (amount <= 0.0)
    {
        printf("[\033[31;1mBank Error\033[0m] Suma insearata nu poate fi negativa! %lf\n", amount);
        return;
    }
    a.balance += (int)(amount * 100);
    write_account(&a);
    printf("[\033[92;1mBank\033[0m] Inserarea a avut loc cu succes! Noua balanta: \033[97;1m%.2f\033[0m \n", (double)(a.balance / 100.00));
}

void withdraw_account_money(struct Account a, double amount)
{
    if (amount >= 0.0)
    {
        printf("[\033[31;1mBank Error\033[0m] Suma scoasa nu poate fi pozitiva! %lf\n", amount);
        return;
    }

    if (a.balance + (int)(amount * 100) < 0)
    {
        printf("[\033[31;1mBank Error\033[0m] Suma scoasa nu poate fi mai mare decat suma contului ! %lf\n", amount);
        return;
    }

    a.balance += (int)(amount * 100);
    write_account(&a);
    printf("[\033[92;1mBank\033[0m] Scoaterea a avut loc cu succes! Noua balanta: \033[97;1m%.2f\033[0m \n", (double)(a.balance / 100.00));
}

void withdraw_money(char *iden, char *uid, double amount)
{
    if (strcmp(iden, "id") == 0)
    {
        if (!isdigit(uid[0]))
        {
            printf("[\033[31;1mBank Error\033[0m] Te rugam introdu un id valid! [1..]\n");
            return;
        }

        int _id = atoi(uid);
        sprintf(uid, "./accounts/%d.acc", _id);

        FILE *f = fopen(uid, "r+");
        if (f == NULL)
        {
            printf("[\033[31;1mBank Error\033[0m] id-ul %d nu exista!\n", _id);
            return;
        }
        struct Account a;
        parse_account(&a, uid, f);
        withdraw_account_money(a, amount);
        fclose(f);
    }
    else if (strcmp(iden, "cont") == 0)
    {
        struct dirent *f;
        DIR *dir = open_account_dir();
        if (dir == NULL)
        {
            printf("[\033[31;1mBank Error\033[0m] Nu exista un fisier pentru conturi(accounts). Te rugam creeaza unul.");
            return;
        }

        if (strlen(uid) < 29)
        {
            printf("[\033[31;1mBank Error\033[0m] Te reguam introdu un cont valid!\n");
            return;
        }

        char s[125];
        while ((f = readdir(dir)) != NULL)
        {
            if (f->d_name[0] == '_')
                continue;
            FILE *db;
            sprintf(s, "./accounts/%s", f->d_name);
            db = fopen(s, "r+");
            fgets(s, 120, db);
            if (strstr(s, uid) != NULL)
            {
                struct Account a;

                parse_account(&a, s, db);
                withdraw_account_money(a, amount);
                fclose(db);
                return;
            }
            fclose(db);
        }
    }
    else
    {
        printf("[\033[31;1mBank Error\033[0m] Optiune invalida pentru identificator!\n");
    }
}



void insert_money(char *iden, char *uid, double amount)
{
    if (strcmp(iden, "id") == 0)
    {
        if (!isdigit(uid[0]))
        {
            printf("[\033[31;1mBank Error\033[0m] Te rugam introdu un id valid! [1..]\n");
            return;
        }

        int _id = atoi(uid);
        sprintf(uid, "./accounts/%d.acc", _id);

        FILE *f = fopen(uid, "r+");
        if (f == NULL)
        {
            printf("[\033[31;1mBank Error\033[0m] id-ul \"%d\" nu exista!\n", _id);
            return;
        }
        struct Account a;
        parse_account(&a, uid, f);
        _add_account_money(a, amount);
        fclose(f);
    }
    else if (strcmp(iden, "cont") == 0)
    {
        struct dirent *f;
        DIR *dir = open_account_dir();
        if (dir == NULL)
        {
            printf("[\033[31;1mBank Error\033[0m] Nu exista un fisier pentru conturi(accounts). Te rugam creeaza unul.");
            return;
        }

        if (strlen(uid) < 29)
        {
            printf("[\033[31;1mBank Error\033[0m] Te reguam introdu un cont valid!\n");
            return;
        }

        char s[125];
        while ((f = readdir(dir)) != NULL)
        {
            if (f->d_name[0] == '_')
                continue;
            FILE *db;
            sprintf(s, "./accounts/%s", f->d_name);
            db = fopen(s, "r+");
            fgets(s, 120, db);
            if (strstr(s, uid) != NULL)
            {
                struct Account a;

                parse_account(&a, s, db);
                _add_account_money(a, amount);
                fclose(db);
                return;
            }
            fclose(db);
        }
        printf("[\033[31;1mBank Error\033[0m] Contul \"%s\" nu a fost gasit!\n", uid);
    }
    else
    {
        printf("[\033[31;1mBank Error\033[0m] Optiune invalida pentru identificator!\n");
    }
}

int create_account(char *first_name, char *last_name)
{
    struct Account a;
    a.id = get_last_account_id() + 1;
    a.balance = 0 * CENT;
    generate_random_string(a.iban, 29);

    strcpy(a.first_name, first_name);
    strcpy(a.last_name, last_name);

    write_account(&a);

    printf("\n[\033[92;1mBank\033[0m] Contul a fost creat cu succes. ID: %d\n", get_last_account_id());
    return 0;
}

void delete_account(char *iden, char *uid)
{
    if (strcmp(iden, "id") == 0)
    {
        char aux[100];
        int id = atoi(uid);
        sprintf(aux, "./accounts/%d.acc", id);
        if (remove(aux) == 0)
        {
            printf("[\033[92;1mBank\033[0m] Contul cu id-ul %d a fost sters cu succes!\n", id);
        }
        else
        {
            printf("[\033[31;1mBank Error\033[0m] Contul cu id-ul %d nu exista!\n", id);
        }
    }
    else if (strcmp(iden, "cont") == 0)
    {
        if (strlen(uid) < 29)
        {
            printf("[\033[31;1mBank Error\033[0m] Te reguam introdu un cont valid!\n");
            return;
        }

        struct dirent *f;
        DIR *dir = open_account_dir();
        if (dir == NULL)
        {
            printf("[\033[31;1mBank Error\033[0m] Nu exista un fisier pentru conturi(accounts). Te rugam creeaza unul.");
            return;
        }

        if (strlen(uid) < 29)
        {
            printf("[\033[31;1mBank Error\033[0m] Te reguam introdu un cont valid!\n");
            return;
        }

        char s[125];
        while ((f = readdir(dir)) != NULL)
        {
            if (f->d_name[0] == '_')
                continue;
            FILE *db;
            sprintf(s, "./accounts/%s", f->d_name);
            db = fopen(s, "r+");
            fgets(s, 120, db);
            if (strstr(s, uid) != NULL)
            {
                struct Account a;

                parse_account(&a, s, db);

                sprintf(s, "./accounts/%s", f->d_name);

                if (remove(s) == 0)
                {
                    printf("[\033[92;1mBank\033[0m] Contul cu ibanul %s a fost sters cu succes!\n", a.iban);
                }
                return;
            }
            fclose(db);
        }
        printf("[\033[31;1mBank Error\033[0m] Contul cu ibanul %s nu exista!\n", uid);
    }
    else
    {
        printf("[\033[31;1mBank Error\033[0m] Optiune invalida pentru identificator!\n");
    }
}

void search_account(char *iden, char *uid)
{
    struct dirent *f;
    DIR *dir = open_account_dir();
    if (dir == NULL)
    {
        printf("[\033[31;1mBank Error\033[0m] Nu exista un fisier pentru conturi(accounts). Te rugam creeaza unul.");
        return;
    }

    char s[125];
    while ((f = readdir(dir)) != NULL)
    {
        if (f->d_name[0] == '_')
            continue;
        FILE *db;
        sprintf(s, "./accounts/%s", f->d_name);
        db = fopen(s, "r+");
        fgets(s, 120, db);
        if (strstr(s, uid) != NULL)
        {
            struct Account a;

            parse_account(&a, s, db);

            if (strcmp(iden, "id") == 0 && atoi(uid) == a.id)
            {
                print_account(a);
                fclose(db);
                return;
            }
            if (strcmp(iden, "prenume") == 0 && strcmp(uid, a.first_name) == 0)
            {
                print_account(a);
                fclose(db);
                return;
            }
            if (strcmp(iden, "nume") == 0 && strcmp(uid, a.last_name) == 0)
            {
                print_account(a);
                fclose(db);
                return;
            }
            if (strcmp(iden, "cont") == 0 && strcmp(uid, a.iban) == 0)
            {
                if (strlen(uid) < 29)
                {
                    printf("[\033[31;1mBank Error\033[0m] Te reguam introdu un cont valid!\n");
                    return;
                }
                print_account(a);
                fclose(db);
                return;
            }
        }
    }

    printf("[\033[31;1mBank Error\033[0m] Contul nu fost gasit!\n");
}

void show_menu()
{
    printf("[\033[92;1mBank\033[0m] Aici este lista de comenzi:\n\n");
    printf("creeaza-cont    [char* prenume]                 [char* nume]                   Creeaza un cont bancar pentru un client.\n");
    printf("lista-conturi   [int pagina]        Listeaza conturile bancare prin paginare.\n");
    printf("insereaza-bani  [char* identificator{id, cont}] [double suma]"
           "       Inseara bani intr-un cont bancar dupa numar cont sau id.\n");
    printf("scoate-bani     [char* identificator{id, cont}] [double -suma]"
           "       Scoate bani dintr-un cont bancar dupa numar cont sau id.\n");
    printf("sterge-cont     [char* identificator{id, cont}]"
           "            Sterge un cont bancar dupa numar cont sau id.\n");
    printf("cauta-cont      [char* identificator{id, cont, nume, prenume}] [char* valoare]"
           "            Cauta un cont bancar dupa un identificator.\n");
}

int main(int argc, char **argv)
{
    srand(time(NULL));
    struct stat st = {0};
    if (stat("./accounts", &st) == -1)
    {
        mkdir("./accounts", 0700);
        FILE *f = fopen("./accounts/_last_inserted_id", "w");
        fprintf(f, "0");
        fclose(f);
    }
    system(CLEAR_CONSOLE);
    if (argc < 2)
    {
        show_menu();
        return 0;
    }

    if (strcmp(*(argv + 1), "help") == 0)
    {
        show_menu();
    }
    else if (strcmp(*(argv + 1), "lista-conturi") == 0)
    {
        if (argc < 3)
        {
            printf("[\033[31;1mBank Error\033[0m] Te rugam introdu un numar pentru paginare!\n");
            return 1;
        }
        int page = atoi(*(argv + 2));
        if (page <= 0)
        {
            printf("[\033[31;1mBank Error\033[0m] Te rugam introdu un numar valid! [1..n]\n");
            return 1;
        }
        printf("[\033[92;1mBank\033[0m] Aici sunt conturile bancare ale clientilor:\n");
        list_accounts(page);
    }
    else if (strcmp(*(argv + 1), "creeaza-cont") == 0)
    {
        if (argc < 4)
        {
            printf("[\033[31;1mBank Error\033[0m] Te rugam introdu toate datele! Exemplu:\n./program creeaza-cont {prenume} {nume}\n");
            return 0;
        }
        create_account(*(argv + 2), *(argv + 3));
    }
    else if (strcmp(*(argv + 1), "sterge-cont") == 0)
    {
        if (argc < 4)
        {
            printf("[\033[31;1mBank Error\033[0m] Te rugam introdu toate datele! Exemplu:\n./program sterge-cont {iden: {cont, id}} {valoare}\n");
            return 0;
        }
        delete_account(*(argv + 2), *(argv + 3));
    }
    else if (strcmp(*(argv + 1), "cauta-cont") == 0)
    {
        if (argc < 4)
        {
            printf("[\033[31;1mBank Error\033[0m] Te rugam introdu toate datele! Exemplu:\n./program cauta-cont {iden: {nume, prenume, id, cont}} {valoare}\n");
            return 0;
        }
        search_account(*(argv + 2), *(argv + 3));
    }
    else if (strcmp(*(argv + 1), "insereaza-bani") == 0)
    {
        if (argc < 5)
        {
            printf("[\033[31;1mBank Error\033[0m] Te rugam introdu toate datele! Exemplu:\n./program insereaza-bani {id, cont} {uid} 20\n");
            return 0;
        }
        insert_money(*(argv + 2), *(argv + 3), atof(*(argv + 4)));
    }
    else if (strcmp(*(argv + 1), "scoate-bani") == 0)
    {
        if (argc < 5)
        {
            printf("[\033[31;1mBank Error\033[0m] Te rugam introdu toate datele! Exemplu:\n./program scoate-bani {id, cont} {uid} -20\n");
            return 0;
        }
        withdraw_money(*(argv + 2), *(argv + 3), atof(*(argv + 4)));
    }
    else
    {
        printf("[\033[31;1mBank Error\033[0m] Aceasta comanda inca nu a fost implementata!\n");
    }
}