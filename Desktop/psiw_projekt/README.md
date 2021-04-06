# IPC Publish - Subscribe
## Ogólne informacje
Celem projektu jest implementacja systemu rozgłaszania wiadomości do wszystkich procesów, które zasubskrybowały dany typ wiadomości. Tworzona aplikacja wykorzystuje mechanizm kolejek komunikatów. W ramach projektu zostały opracowane programy klienta i serwera oraz protokół komunikacyjny. Został użyty język C.

### 1. Instrukcja kompilacji
Aby skompilować pliki projektu należy wpisać osobno w terminalu poniższe linie i po każdej z nich kliknąć Enter. Pliki powinny skompilować się bez ostrzeżeń.

gcc -Wall serwer.c -o serwer

gcc -Wall klient.c -o klient

### 2. Instrukcja uruchamiania
Aby uruchomić projekt należy otworzyć przynajmniej 2 okna terminalu. W jednym z okien należy uruchomić program serwera poprzez wpisanie: 

./serwer

Aby uruchomić program klienta należy w nowym oknie terminalu wpisać:

./klient

Następnie śledzić instrukcje podane na ekranie. Aby zamknąć programy należy wcisnąć Ctrl+C.

### 3. Opis zawartości pliku serwer.c
Jest to program serwera odbierający informacje od klientów i rozgłaszający wiadomości.
* Funkcja main

* Funkcja wyslij_do_subskrybentow

* Funkcja zarejestruj_odbiorce_typu

* Funkcja wyswietl_typy_klienta

* Funkcja zaloguj_klienta

* Funkcja wyswietl_zalogowanych_klientow
  
* Funkcja wyslij_informacje_o_nowym_typie

* Funkcja dodaj_typ

* Funkcja porownaj_typy
  
* Funkcja wyswietl_typy

* struktury do przechowywania informacji:
  Typ_Subskrypcja, Klient, Wiadomosc, Typ_Wiadomosci


### 4. Opis zawartości pliku klient.c

Jest to program klienta, który wysyła i odbiera wiadomości od serwera.

* Funkcja main
  
* struktury do przechowywania informacji: Typ_Subskrypcja, Klient, Wiadomosc

