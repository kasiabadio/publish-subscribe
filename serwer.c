#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdbool.h>

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

// struktura do przechowywania typu oraz subskrybentów tego typu
struct Typ_Wiadomosci
{
    int typ_wiadomosci;
    int odbiorcy_typu[256];
    int liczba_odbiorcow;
    int rodzaje_subskrypcji_typu[256];
};

// wyświetlanie typów z bazy typów
void wyswietl_typy(struct Typ_Wiadomosci baza_typow[], size_t liczba_typow)
{
    int i = 0;
    int j = 0;
    printf("TYPY Z BAZY TYPÓW\n\n");
    if (liczba_typow == 0)
    {
        printf("--->Brak typów w bazie typów.\n\n");
        return;
    }
    while (i < liczba_typow)
    {
        printf("Typ wiadomości: %d \n", baza_typow[i].typ_wiadomosci);
        printf("Odbiorcy typu: ");
        int k = 0;
        while (k < baza_typow[i].liczba_odbiorcow)
        {
            printf("%d ", baza_typow[i].odbiorcy_typu[k]);
            k++;
        }
        printf("\n");
        printf("Subskrypcje:   ");
        int r = 0;
        while (r < baza_typow[i].liczba_odbiorcow)
        {
            printf("%d ", baza_typow[i].rodzaje_subskrypcji_typu[r]);
            r++;
        }
        printf("\n\n");
        j++;
        i++;
    }
    printf("\n");
}

// porównywacz do posortowania bazy typów
int porownaj_typy(const void *w1, const void *w2)
{
    struct Typ_Wiadomosci *temp1 = (struct Typ_Wiadomosci *)w1;
    struct Typ_Wiadomosci *temp2 = (struct Typ_Wiadomosci *)w2;
    if (temp1->typ_wiadomosci == temp2->typ_wiadomosci)
    {
        return 0;
    }
    else if (temp1->typ_wiadomosci > temp2->typ_wiadomosci)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

// dodanie typu do bazy jeśli go tam nie było
int dodaj_typ(struct Typ_Wiadomosci baza_typow[], size_t liczba_typow, int _typ_wiadomosci)
{
    // sprawdzenie czy typ wiadomosci jest w bazie
    int i = 0;
    while (i < liczba_typow)
    {
        if (baza_typow[i].typ_wiadomosci == _typ_wiadomosci)
        {
            return 1;
        }
        i++;
    }
    // dodanie typu do bazy, jeśli wcześniej go nie było
    baza_typow[i].typ_wiadomosci = _typ_wiadomosci;
    liczba_typow++;
    qsort(baza_typow, liczba_typow, sizeof(baza_typow[0]), porownaj_typy);
    return 0;
}

// rozgłoszenie wiadomości o nowym typie jeśli go tam jeszcze nie ma
int wyslij_informacje_o_nowym_typie(struct Klient zalogowani_klienci[], size_t liczba_klientow, struct Klient klient, int czy_typ_dodany)
{

    if (czy_typ_dodany == 1)
    {
        int msgid_klienta = msgget(klient.id_klienta, IPC_CREAT | IPC_EXCL | 0600);
        if (msgid_klienta == -1)
        {
            msgid_klienta = msgget(klient.id_klienta, IPC_CREAT | 0600);
        }
        //printf("Kliencie o id %d, typ nr %d już istnieje.\n", klient.id_klienta, klient.nowy_typ_rejestracja);
        // wyslij błąd do procesu rejestrującego nowy typ
        klient.m_typ = 3;
        strcpy(klient.informacja_o_typie, "Typ istnieje.\n");
        msgsnd(msgid_klienta, &klient, sizeof(struct Klient) - sizeof(long), 0);
        klient.nowy_typ_rejestracja = 0;
        return 0;
    }

    // wysłanie przez serwer informacji o nowym typie do wszystkich zalogowanych użytkowników
    int index_zalogowani_klienci = 0;

    while (index_zalogowani_klienci < liczba_klientow)
    {

        int msgid_klienta = msgget(zalogowani_klienci[index_zalogowani_klienci].id_klienta, IPC_CREAT | IPC_EXCL | 0600);
        if (msgid_klienta == -1)
        {
            msgid_klienta = msgget(zalogowani_klienci[index_zalogowani_klienci].id_klienta, IPC_CREAT | 0600);
        }
        char informacja_o_typie[64];
        snprintf(informacja_o_typie, sizeof(informacja_o_typie), "Utworzono typ nr %d\n", klient.nowy_typ_rejestracja);
        klient.m_typ = 3;
        strcpy(klient.informacja_o_typie, informacja_o_typie);

        struct Klient kl;
        kl.m_typ = 3;
        strcpy(kl.informacja_o_typie, informacja_o_typie);
        msgsnd(msgid_klienta, &kl, sizeof(struct Klient) - sizeof(long), 0);

        index_zalogowani_klienci++;
    }

    return 0;
}

// wyswietlanie klientow z bazy klientow
void wyswietl_zalogowanych_klientow(struct Klient zalogowani_klienci[], size_t liczba_klientow)
{
    int i = 0;
    printf("\nZALOGOWANI KLIENCI\n");
    while (i < liczba_klientow)
    {
        printf("Id: %d, Nazwa: %s", zalogowani_klienci[i].id_klienta, zalogowani_klienci[i].nazwa_klienta);
        i++;
        printf("\n");
    }
    printf("\n");
}

// dodanie klienta do bazy klientów = logowanie
int zaloguj_klienta(struct Klient zalogowani_klienci[], size_t liczba_klientow, struct Klient klient)
{
    // sprawdzenie czy id klienta jest w bazie zalogowanych
    int i = 0;
    while (i < liczba_klientow)
    {
        if (zalogowani_klienci[i].id_klienta == klient.id_klienta)
        {
            printf("Id: %d jest już zalogowany\n", klient.id_klienta);
            return 1;
        }
        i++;
    }

    // dodanie klienta do bazy zalogowanych
    printf("--->Zalogowano klienta: %d %s\n", klient.id_klienta, klient.nazwa_klienta);
    zalogowani_klienci[i].id_klienta = klient.id_klienta;
    strcpy(zalogowani_klienci[i].nazwa_klienta, klient.nazwa_klienta);

    return 0;
}

// wyświetlanie typów klienta
void wyswietl_typy_klienta(struct Klient klient)
{
    int i = 0;
    printf("KLIENT\n");
    printf("nazwa_klienta: %s  id_klienta: %d \n", klient.nazwa_klienta, klient.id_klienta);
    printf("subskrybuje:         ");
    while (i < klient.liczba_typow)
    {
        printf("%d\t", klient.typy_oraz_rodzaje_subskrypcji_klienta[i].typ_wiadomosci);
        i++;
    }
    printf("\nrodzaje subskrypcji: ");
    int j = 0;
    while (j < klient.liczba_typow)
    {
        printf("%d\t", klient.typy_oraz_rodzaje_subskrypcji_klienta[j].rodzaj_subskrypcji_typu);
        j++;
    }
    printf("\n\n");
}

// rejestracja odbiorcy do bazy odbiorców typów i zapisanie rodzaju subskrypcji jaką wybrał
int zarejestruj_odbiorce_typu(struct Typ_Wiadomosci baza_typow[], size_t liczba_typow, struct Klient odbiorca)
{

    //wyswietl_typy_klienta(odbiorca);
    // posortuj tablicę stuktur typ-subskrypcja klienta
    qsort(odbiorca.typy_oraz_rodzaje_subskrypcji_klienta, odbiorca.liczba_typow, sizeof(odbiorca.typy_oraz_rodzaje_subskrypcji_klienta[0]), porownaj_typy);
    wyswietl_typy_klienta(odbiorca);

    // przejdź przez bazę typów i dodaj tam klienta jeśli nie jest on dopisany
    int index_baza_typow = 0;
    int index_baza_klienta = 0;

    while (index_baza_typow < liczba_typow)
    { // sprawdzanie, czy znalazło kolejny szukany rodzaj typu z bazy klienta
        if (baza_typow[index_baza_typow].typ_wiadomosci == odbiorca.typy_oraz_rodzaje_subskrypcji_klienta[index_baza_klienta].typ_wiadomosci)
        {

            // rodzaj subskrypcji typu
            int temp_subskrypcja = odbiorca.typy_oraz_rodzaje_subskrypcji_klienta[index_baza_klienta].rodzaj_subskrypcji_typu;

            // sprawdzanie czy w bazie odbiorców danego typu znajduje się aktualny odbiorca
            int index_baza_odbiorcow_typu = 0;
            int czy_odbiorca_juz_sie_znajduje = false;
            while ((index_baza_odbiorcow_typu < baza_typow[index_baza_typow].liczba_odbiorcow) && (czy_odbiorca_juz_sie_znajduje == false))
            {

                // jeśli znalazło, że odbiorca jest w bazie odbiorców danego typu, to tam zmień rodzaj subskrypcji na nowy
                if (baza_typow[index_baza_typow].odbiorcy_typu[index_baza_odbiorcow_typu] == odbiorca.id_klienta)
                {

                    baza_typow[index_baza_typow].rodzaje_subskrypcji_typu[index_baza_odbiorcow_typu] = temp_subskrypcja;
                    czy_odbiorca_juz_sie_znajduje = true;
                }
                index_baza_odbiorcow_typu++;
            }
            // jeżeli w bazie odbiorców danego typu nie ma aktualnego odbiorcy, to go dodajemy
            if (czy_odbiorca_juz_sie_znajduje == false)
            {

                // biorę indeks w bazie typów aktualnego elementu i tam dopisuje odbiorcę typu na odpowiedniej pozycji
                int temp_id = odbiorca.id_klienta;
                // dopisuję id nowego odbiorcy danego typu na końcu bazy odbiorców tego typu
                baza_typow[index_baza_typow].odbiorcy_typu[baza_typow[index_baza_typow].liczba_odbiorcow] = temp_id;
                // biorę indeks w bazie typów aktualnego elementu i tam dopisuję rodzaj subskrypcji dla tego typu na odpowiedniej pozycji
                int temp_subskrypcja = odbiorca.typy_oraz_rodzaje_subskrypcji_klienta[index_baza_klienta].rodzaj_subskrypcji_typu;
                baza_typow[index_baza_typow].rodzaje_subskrypcji_typu[baza_typow[index_baza_typow].liczba_odbiorcow] = temp_subskrypcja;
                // zwiększenie liczby odbiorców tego typu
                baza_typow[index_baza_typow].liczba_odbiorcow++;
            }

            index_baza_klienta++;
        }
        index_baza_typow++;
    }

    return 0;
}

// szukanie typu i wysyłanie wiadomości do subskrybentów tego typu
int wyslij_do_subskrybentow(struct Typ_Wiadomosci baza_typow[], size_t liczba_typow, struct Wiadomosc wiadomosc)
{

    int index_baza_typow = 0;
    bool flag = false;
    while (index_baza_typow < liczba_typow)
    {
        // znalazło typ, więc patrzę jakich ma subskrybentów i  wysyłam wiadomość
        if (baza_typow[index_baza_typow].typ_wiadomosci == wiadomosc.typ_wiadomosci)
        {
            flag = true;
            int index_odbiorcy_typu = 0;
            while (index_odbiorcy_typu < baza_typow[index_baza_typow].liczba_odbiorcow)
            {
                int id_klienta = baza_typow[index_baza_typow].odbiorcy_typu[index_odbiorcy_typu];

                // jeśli wartość subskrypcji jest równa 0, to wysyłam od razu wiadomość
                if (baza_typow[index_baza_typow].rodzaje_subskrypcji_typu[index_odbiorcy_typu] == 0)
                {
                    // wyślij wiadomość
                    int msgid_klienta = msgget(id_klienta, IPC_CREAT | IPC_EXCL | 0600);
                    if (msgid_klienta == -1)
                    {
                        msgid_klienta = msgget(id_klienta, IPC_CREAT | 0600);
                    }
                    // wyślij wiadomość
                    wiadomosc.m_typ = 1;
                    msgsnd(msgid_klienta, &wiadomosc, sizeof(struct Wiadomosc) - sizeof(long), 0);

                    // jeśli wartość subskrypcji po dekrementacji osiągnie 0, to wysyłam wiadomość zmieniam wartość subskrypcji na -1
                }
                else if (baza_typow[index_baza_typow].rodzaje_subskrypcji_typu[index_odbiorcy_typu] - 1 == 0)
                {

                    baza_typow[index_baza_typow].rodzaje_subskrypcji_typu[index_odbiorcy_typu] = -1;
                    int msgid_klienta = msgget(id_klienta, IPC_CREAT | IPC_EXCL | 0600);
                    if (msgid_klienta == -1)
                    {
                        msgid_klienta = msgget(id_klienta, IPC_CREAT | 0600);
                    }
                    // wyślij wiadomość
                    wiadomosc.m_typ = 1;
                    msgsnd(msgid_klienta, &wiadomosc, sizeof(struct Wiadomosc) - sizeof(long), 0);

                    // jeśli wartość subskrypcji > 0 i nie osiągnie 0 po dekrementacji, to wysyłam wiadomość i dekrementuję subskrypcję
                }
                else if (baza_typow[index_baza_typow].rodzaje_subskrypcji_typu[index_odbiorcy_typu] > 0)
                {

                    baza_typow[index_baza_typow].rodzaje_subskrypcji_typu[index_odbiorcy_typu] -= 1;
                    int msgid_klienta = msgget(id_klienta, IPC_CREAT | IPC_EXCL | 0600);
                    if (msgid_klienta == -1)
                    {
                        msgid_klienta = msgget(id_klienta, IPC_CREAT | 0600);
                    }
                    // wyślij wiadomość
                    wiadomosc.m_typ = 1;
                    msgsnd(msgid_klienta, &wiadomosc, sizeof(struct Wiadomosc) - sizeof(long), 0);
                }
                index_odbiorcy_typu++;
            }
        }
        index_baza_typow++;
    }

    if (flag == false)
    {
        printf("--->Error: Brak typu %d w bazie\n", wiadomosc.typ_wiadomosci);
        printf("--->Nie mozna przeslac wiadomosci o tresci: %s\n\n", wiadomosc.tresc_wiadomosci);
    }

    return 0;
}


int main(int argc, char *argv[])
{

    // utworzenie kolejki komunikatów do której klienci zapisują a serwer z niej odbiera
    int msgid = msgget(81325, IPC_CREAT | IPC_EXCL | 0600);
    if (msgid == -1)
    {
        msgid = msgget(81325, IPC_CREAT | 0600);
    }

    // do logowania
    struct Klient zalogowani_klienci[1000];
    size_t liczba_klientow = 0;
    struct Klient klient;
    // do rejestracji typu
    struct Typ_Wiadomosci baza_typow[1000];
    size_t liczba_typow = 0;
    wyswietl_typy(baza_typow, liczba_typow);
    struct Wiadomosc wiadomosc;

    // odbieranie wszystkich wiadomosci z flagą IPC_NOWAIT
    while (1)
    {
        // LOGOWANIE
        // serwer otrzymuje od klienta jego id, nazwę oraz msgid_klienta i dopisuje do systemu do bazy użytkowników
        if (msgrcv(msgid, &klient, sizeof(struct Klient) - sizeof(long), 5, IPC_NOWAIT) > 0)
        {
            int czy_klient_dodany = zaloguj_klienta(zalogowani_klienci, liczba_klientow, klient);
            if (czy_klient_dodany == 0)
            {
                liczba_klientow++;
            }
            wyswietl_zalogowanych_klientow(zalogowani_klienci, liczba_klientow);
        }
        // REJESTRACJA TYPU
        // serwer przechowuje informację o wszystkich możliwych typach wiadomości
        if (msgrcv(msgid, &klient, sizeof(struct Klient) - sizeof(long), 4, IPC_NOWAIT) > 0)
        {
            int czy_typ_dodany = dodaj_typ(baza_typow, liczba_typow, klient.nowy_typ_rejestracja);
            if (czy_typ_dodany == 0)
            {
                liczba_typow++;
            }
            // wysyłanie informacji o nowym typie
            wyslij_informacje_o_nowym_typie(zalogowani_klienci, liczba_klientow, klient, czy_typ_dodany);
            
            wyswietl_typy(baza_typow, liczba_typow);
        }
        // REJESTRACJA ODBIORCY
        // serwer odczytuje typy wiadomości, które klient chciałby otrzymać w przyszłości
        if (msgrcv(msgid, &klient, sizeof(struct Klient) - sizeof(long), 2, IPC_NOWAIT) > 0)
        {
            int czy_typ_dodany1 = dodaj_typ(baza_typow, liczba_typow, klient.nowy_typ_rejestracja);
            if (czy_typ_dodany1 == 0)
            {
                liczba_typow++;
            }
            wyslij_informacje_o_nowym_typie(zalogowani_klienci, liczba_klientow, klient, czy_typ_dodany1);
            zarejestruj_odbiorce_typu(baza_typow, liczba_typow, klient);
            wyswietl_typy(baza_typow, liczba_typow);
            
        }
        // ODBIÓR WIADOMOŚCI
        // serwer odbiera wiadomość od klienta, odczytuje treść, typ i priorytet

        if (msgrcv(msgid, &wiadomosc, sizeof(struct Wiadomosc) - sizeof(long), 1, IPC_NOWAIT) > 0)
        {
            printf(">>Odebrano wiadomość: %s\n\n", wiadomosc.tresc_wiadomosci);
            // wysyłanie wiadomości do subskrybentów jej typu
            wyslij_do_subskrybentow(baza_typow, liczba_typow, wiadomosc);
            wyswietl_typy(baza_typow, liczba_typow);
        }

    }

    // usuwanie kolejek klientów
    for (int j = 0; j < liczba_klientow; j++)
    {
        msgctl(zalogowani_klienci[j].id_klienta, IPC_RMID, NULL);
    }

    return 0;
}