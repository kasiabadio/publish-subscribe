#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>


struct Typ_Subskrypcja{
    int typ_wiadomosci;
    int rodzaj_subskrypcji_typu;

};


struct Klient{
    long m_typ;
    int id_klienta; //każdy klient podaje to id przy logowaniu
    char nazwa_klienta [64]; // -||- przy logowaniu
    struct Typ_Subskrypcja typy_oraz_rodzaje_subskrypcji_klienta [128];
    int liczba_typow;
    
    int nowy_typ_rejestracja;
    char informacja_o_typie[64];
    int msgid_klienta;
};


// struktura do przechowywania treści, priorytetu wiadomości i typu wiadomości
struct Wiadomosc{
    long m_typ;
    char tresc_wiadomosci[128];
    int priorytet_wiadomosci;
    int typ_subskrypcji;
    int typ_wiadomosci;

};


int main(int argc, char* argv[]){

    // utworzenie kolejki komunikatów do której klienci zapisują a serwer z niej odbiera
    int msgid = msgget(81325, IPC_CREAT|IPC_EXCL|0600);
    if(msgid == -1){
        msgid = msgget(81325, IPC_CREAT|0600);
    }

    // LOGOWANIE
    // klient loguje się do systemu podając id i nazwę
    struct Klient klient;

    int msgid_klienta = msgget(11123, IPC_CREAT|IPC_EXCL|0600);
    if(msgid_klienta == -1){
        msgid_klienta = msgget(11123, IPC_CREAT|0600);
    }

    klient.m_typ = 5;
    klient.id_klienta = 29383;
    klient.liczba_typow = 2;
    strcpy(klient.nazwa_klienta, "Kasia");
    msgsnd(msgid, &klient, sizeof(struct Klient) - sizeof(long), 0);
    
    // REJESTRACJA TYPU/ (TYPÓW?)
    // 9, 2, 19, 191, 1
    // klient żąda utworzenia nowego typu wiadomości przez system
    // klient podaje: nowy_typ_rejestracja
    // przekazane będzie: struktura Klient
    for(int i = 10; i > 0; i--){
        klient.nowy_typ_rejestracja = i;
        klient.m_typ = 4;
        msgsnd(msgid, &klient, sizeof(struct Klient) - sizeof(long), 0);
        msgrcv(msgid_klienta, &klient, sizeof(struct Klient) - sizeof(long), 3, 0);
        printf("Odebrano: %s\n", klient.informacja_o_typie);
    }


    // REJESTRACJA ODBIORCY
    // klient podaje: [nazwę, (id?), typy, rodzaje subskrypcji typów] = struct Klient
    klient.m_typ = 2;
    klient.typy_oraz_rodzaje_subskrypcji_klienta[0].typ_wiadomosci = 9;
    klient.typy_oraz_rodzaje_subskrypcji_klienta[0].rodzaj_subskrypcji_typu = 2;
    klient.liczba_typow = 1;
    msgsnd(msgid, &klient, sizeof(struct Klient) - sizeof(long), 0);

    
    // ROZGŁASZANIE NOWEJ WIADOMOŚCI
    // klient wysyła treść rozgłaszanej wiadomości wraz z jej typem i priorytetem
    // klient podaje: [treść, priorytet, typ] = struct Wiadomosc
    struct Wiadomosc wiadomosc;
    struct Wiadomosc wiadomosc1;
    for(int i = 0; i < 2; i++){
        wiadomosc.m_typ = 1;
        strcpy(wiadomosc.tresc_wiadomosci, "Hello World!\n");
        wiadomosc.priorytet_wiadomosci = 1;
        wiadomosc.typ_wiadomosci = 9;
        msgsnd(msgid, &wiadomosc,  sizeof(struct Wiadomosc) - sizeof(long), 0);
        msgrcv(msgid_klienta, &wiadomosc1, sizeof(struct Wiadomosc) - sizeof(long), 1, 0);
        printf("Odebrano wiadomość: %s\n", wiadomosc1.tresc_wiadomosci);

    }
    
    
    // ODBIÓR WIADOMOŚCI
    // odbiór synchroniczny
    // klient odbiera wiadomość danego typu przez wywołanie funkcji
    // jeśli wiadomość nie jest jeszcze dostępna to wywołanie jest blokujące (?)
    // wyświetlenie wiadomości na ekranie


    return 0;
}