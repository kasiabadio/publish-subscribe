#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>

struct Typ_Subskrypcja
{
    int typ_wiadomosci;
    int rodzaj_subskrypcji_typu;
};

struct Klient
{
    long m_typ;
    int id_klienta;         //każdy klient podaje to id przy logowaniu
    char nazwa_klienta[64]; // -||- przy logowaniu
    struct Typ_Subskrypcja typy_oraz_rodzaje_subskrypcji_klienta[128];
    int liczba_typow;

    int nowy_typ_rejestracja;
    char informacja_o_typie[64];
};

// struktura do przechowywania treści i typu wiadomości
struct Wiadomosc
{
    long m_typ;
    char tresc_wiadomosci[128];
    int typ_wiadomosci;
};

int main(int argc, char *argv[])
{

    // utworzenie kolejki komunikatów do której klienci zapisują a serwer z niej odbiera
    int msgid = msgget(81325, IPC_CREAT | IPC_EXCL | 0600);
    if (msgid == -1)
    {
        msgid = msgget(81325, IPC_CREAT | 0600);
    }

    // LOGOWANIE  -> jednokrotne
    // klient loguje się do systemu podając id i nazwę
    struct Klient klient;
    // tworzenie kolejki komunikatów klienta do której zapisuje serwer
    // generowanie losowego id klienta
    srand(time(0));
    int dolne = 10000;
    int gorne = 100000;
    klient.id_klienta = (rand() % (gorne - dolne + 1)) + dolne;
    printf("Generated: %d\n", klient.id_klienta);
    int msgid_klienta = msgget(klient.id_klienta, IPC_CREAT | IPC_EXCL | 0600);

    // jeżeli kolejka o takim kluczu istnieje to wylosuj id na nowo
    while (msgid_klienta == -1)
    {
        fputs("--->Error: Kolejka o tym kluczu już istnieje.\n", stdout);
        fputs("Generowanie nowego klucza ...\n", stdout);
        klient.id_klienta = (rand() % (gorne - dolne + 1)) + dolne;
        printf("Generated: %d\n", klient.id_klienta);
        msgid_klienta = msgget(klient.id_klienta, IPC_CREAT | IPC_EXCL | 0600);
    }
    
    // uzupełnienie danych klienta, pobranie jego nazwy
    klient.m_typ = 5;
    klient.liczba_typow = 0;
    fputs("Podaj swoja nazwe klienta: ", stdout);
    fgets(klient.nazwa_klienta, 64, stdin);
    // wysyłanie danych klienta do kolejki serwera
    msgsnd(msgid, &klient, sizeof(struct Klient) - sizeof(long), 0);
    int pid;
    if ((pid = fork()) != 0)
    { // wysyłanie wiadomości

        int opcja;
        while (1)
        {
            // REJESTRACJA TYPU
            // klient żąda utworzenia nowego typu wiadomości przez system
            // klient podaje: nowy_typ_rejestracja
            // przekazane będzie: struktura Klient
            fputs("1 - rejestracja typu \n2 - rejestracja odbiorcy typu\n3 - wyslanie wiadomosci\n", stdout);
            scanf("%d", &opcja);
            if (opcja == 1)
            {
                fputs("Podaj typ do rejestracji (liczba całkowita dodatnia): ", stdout);
                scanf("%d", &klient.nowy_typ_rejestracja);
                klient.m_typ = 4;
                msgsnd(msgid, &klient, sizeof(struct Klient) - sizeof(long), 0);
            }

            // REJESTRACJA ODBIORCY
            // klient podaje: [nazwę, (id?), typ, rodzaj subskrypcji typu] = struct Klient
            if (opcja == 2)
            {
                klient.m_typ = 2;
                fputs("Podaj typ ktory chcesz subskrybowac (liczba całkowita dodatnia): ", stdout);
                scanf("%d", &klient.nowy_typ_rejestracja);
                // sprawdź czy klient już subskrybuje ten typ
                int index_typy_klienta = 0;
                int czy_jest_typ = false;
                while (index_typy_klienta < klient.liczba_typow && czy_jest_typ == false)
                {
                    if (klient.typy_oraz_rodzaje_subskrypcji_klienta[index_typy_klienta].typ_wiadomosci == klient.nowy_typ_rejestracja)
                    {
                        czy_jest_typ = true;
                        fputs("Podaj rodzaj subskrypcji tego typu (liczba całkowita dodatnia): ", stdout);
                        scanf("%d", &klient.typy_oraz_rodzaje_subskrypcji_klienta[index_typy_klienta].rodzaj_subskrypcji_typu);
                    }
                    index_typy_klienta++;
                }
                if (czy_jest_typ == false)
                {
                    klient.typy_oraz_rodzaje_subskrypcji_klienta[klient.liczba_typow].typ_wiadomosci = klient.nowy_typ_rejestracja;
                    fputs("Podaj rodzaj subskrypcji tego typu (liczba całkowita dodatnia): ", stdout);
                    scanf("%d", &klient.typy_oraz_rodzaje_subskrypcji_klienta[klient.liczba_typow].rodzaj_subskrypcji_typu);
                    klient.liczba_typow++;
                }

                msgsnd(msgid, &klient, sizeof(struct Klient) - sizeof(long), 0);
            }

            // ROZGŁASZANIE NOWEJ WIADOMOŚCI
            // klient wysyła treść rozgłaszanej wiadomości wraz z jej typem i priorytetem
            // klient podaje: [treść, priorytet, typ] = struct Wiadomosc
            if (opcja == 3)
            {
                struct Wiadomosc wiadomosc;
                char temp;
                wiadomosc.m_typ = 1;
                fputs("Podaj typ wiadomosci (liczba całkowita dodatnia): ", stdout);
                scanf("%d", &wiadomosc.typ_wiadomosci);
                scanf("%c", &temp);
                fputs("Podaj tresc wiadomosci: \n", stdout);
                scanf("%[^\n]", wiadomosc.tresc_wiadomosci);
                msgsnd(msgid, &wiadomosc, sizeof(struct Wiadomosc) - sizeof(long), 0);
            }

 
        }
    }
    else
    { //odbieranie wiadomości
        
        while (1)
        {   
            struct Klient klient1;
            // ODBIÓR WIADOMOŚCI O ZAREJESTROWANYCH TYPACH
            if (msgrcv(msgid_klienta, &klient1, sizeof(struct Klient) - sizeof(long), 3, IPC_NOWAIT) > 0)
            {
                printf(">>Odebrano: %s\n", klient1.informacja_o_typie);
            }

            // ODBIÓR WIADOMOŚCI Z SYSTEMU ROZGŁASZANIA
            struct Wiadomosc wiadomosc1;
            if(msgrcv(msgid_klienta, &wiadomosc1, sizeof(struct Wiadomosc) - sizeof(long), 1, IPC_NOWAIT) > 0){
                printf(">>Odebrano wiadomość: %s\n", wiadomosc1.tresc_wiadomosci);
            }

        }
    }

    // usuwanie kolejki klienta
    msgctl(klient.id_klienta, IPC_RMID, NULL);

    return 0;
}