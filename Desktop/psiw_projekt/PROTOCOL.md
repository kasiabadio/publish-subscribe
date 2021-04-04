# IPC Publish - Subscribe
## Ogólne informacje
    Celem projektu jest implementacja systemu rozgłaszania wiadomości do wszystkich procesów, które zasubskrybowały dany typ wiadomości. Tworzona aplikacja wykorzystuje mechanizm kolejek komunikatów. W ramach projektu zostały opracowane programy klienta i serwera oraz protokół komunikacyjny. Został użyty język C.

### 1. Użyte w projekcie struktury
Obecne w programie serwera oraz klienta:
* struct Klient : służy do logowania klienta, rejestracji nowego typu wiadomości, rejestracji do subskrybowania nowego typu wiadomości oraz odbierania informacji od serwera o powodzeniu tworzenia nowego typu (oraz informacji o utworzonych nowych typach)
  
  Obecne pola struktury:

    * long m_typ - umożliwia odbieranie wiadomości danego typu przez serwer
    * int id_klienta - identyfikator klienta, wygenerowany losowo, przy okazji tworzenia kolejki komunikatów należącej do klienta
    * char nazwa_klienta[64] - nazwa podawana przez klienta przy logowaniu
    * struct Typ_Subskrypcja typy_oraz_rodzaje_subskrypcji_klienta[128];  - tablica przechowująca struktury Typ_Subskrypcja (które przechowują typy subskrypcji klienta oraz rodzaje tych subskrypcji):
    * int liczba_typow - liczba typów, które subskrybuje ten klient (czyli liczba zapełnionych pól w powyższej tablicy)
    * int nowy_typ_rejestracja - numer typu, który klient chce zarejestrować/zasubskrybować
    * char informacja_o_typie[64] - gdy serwer zarejestruje nowy typ (nowy_typ_rejestracja) lub gdy sprawdzi, że taki typ już istnieje, to zapisuje w tej tablicy wiadomość, która zostaje przekazana klientowi

* struct Typ_Subskrypcja
  
    Obecne pola struktury:

    * typ_wiadomosci
    * rodzaj_subskrypcji typu
  
  Umieszczenie w strukturze tych dwóch pól umożliwia powiązanie typu wraz z rodzajem jego subskrypcji.

* struct Wiadomosc :  służy do wymiany wiadomości pomiędzy klientem, serwerem oraz rozgłaszania wiadomości do  subskrybentów
  
  Obecne pola struktury:
  
  * long m_typ - umożliwia odbieranie wiadomości danego typu przez serwer
  * char tresc_wiadomosci[128] - przechowywana jest treść przekazywanej wiadomości 
  * int typ_wiadomosci - przechowywany jest typ przekazywanej wiadomości 

Obecna wyłącznie w programie serwera:

* struct Typ_Wiadomosci : służy do przechowywania wszystkich typów wiadomości wraz z ich subskrybentami
  
  Obecne pola struktury:

    * int typ_wiadomosci - przechowuje typ wiadomości, który jest liczbą całkowitą
    * int odbiorcy_typu[256] - w tej tablicy przechowywane są identyfikatory (id_klienta) klientów, którzy subskrybują ten typ
    * int liczba_odbiorcow - liczba odbiorców, którzy subskrybują ten typ
    * int rodzaje_subskrypcji_typu[256] - w tej tablicy przechowywane są rodzaje subskrypcji typów dla klientów na tych samych pozycjach w tablicy odbiorcy_typu

Ponadto serwer przechowuje dane w tablicach:

struct Klient zalogowani_klienci[1000]; - zalogowani klienci
struct Typ_Wiadomosci baza_typow[1000]; - typy, czyli inaczej tematy wiadomości (dodatkowo w każdym typie są zapisane numery id klientów, którzy subskrybują dane typy oraz rodzaje ich subskrybcji)

### 2. Opis komunikacji pomiędzy komponentami projektu
Komunikacja pomiędzy klientami, a serwerem odbywa się za pomocą kolejek komunikatów. Klienci zapisują informacje do kolejki serwera, która ma odgórnie założony identyfikator (81325). Każdy z klientów tworzy swoją własną kolejkę i przekazuje swój unikalny identyfikator kolejki do serwera, który go odczytuje. Znając ten numer, serwer może przesyłać informacje do klientów.

Serwer odbiera wszystkie wiadomości z flagą IPC_NOWAIT, co zapobiega blokowaniu, gdy nie są dostępne żadne wiadomości danego typu. Jeżeli serwer otrzyma wiadomość z prośbą rejestracji typu, rejestracji odbiorcy i odbioru wiadomości, to od razu po wykonaniu niezbędnych operacji, są wysyłane informacje zwrotne do klienta lub klientów. W przypadku logowania nie jest wysyłana informacja zwrotna do klienta.

W programie klienta wywołana zostaje funkcja fork(), która tworzy nowy proces. Umożliwia to jednoczesne wysyłanie i odbieranie wiadomości.
Klient odbiera wiadomości od serwera w procesie macierzystym, również z flagą IPC_NOWAIT i w efekcie są one wyświetlane na ekranie. Wysyłanie wiadomości jest w procesie macierzystym i następuje w wyniku wybrania jednej z dostępnych opcji przez klienta:
* Opcja 1: rejestracja typu
* Opcja 2: rejestracja odbiorcy
* Opcja 3: rozgłaszanie nowej wiadomości


### 3. Dokładny opis poszczególnych funkcji
### Opis funkcji w pliku serwer.c
Jest to program serwera odbierający informacje od klientów i rozgłaszający wiadomości.
* Funkcja main
  
  - Tworzona jest kolejka serwera.
  - Odbierane są wiadomości różnych typów od klientów z użyciem msgrcv i flagą IPC_NOWAIT: 
  

    Typ 5: odbieranie od klientów struktury klient i logowanie poprzez wywołanie funkcji zaloguj_klienta

    Typ 4: odbieranie od klientów struktury klient i dodanie typu poprzez wywołanie funkcji dodaj_typ, wysyłanie informacji do klientów poprzez wywołanie funkcji wyslij_informacje_o_nowym_typie

    Typ 2: odbieranie od klientów struktury klient i rejestracja ich jako odbiorców danego typu, przy czym jeżeli tego typu nie było w bazie, to jest on do niej dodawany

    Typ 1: odbieranie od klientów struktury wiadomość i wysyłanie jej do subskrybentów tego typu, jeżeli tego typu nie ma w bazie to wiadomość nie jest rozgłaszana
    

* Funkcja wyslij_do_subskrybentow

    Przeszukana zostaje baza_typow w poszukiwaniu typu wiadomości, która ma zostać przesłana. Jeżeli jest on znaleziony, to do odbiorców tego typu wysyłam wiadomość, zgodnie z rodzajem subskrypcji, która jest wartością liczbową:

    * 0: Oznacza, że wiadomości są wysyłane zawsze.

    * Większe niż 0: Oznacza, że wiadomość jest wysyłana, po jej wysłaniu wartość subskrypcji jest dekrementowana o 1. Jeżeli wartość subskrypcji po dekrementacji miałaby osiągnąć wartość 0, to jest ona ustawiana na -1.

* Funkcja zarejestruj_odbiorce_typu
  
  Początkowo sortowane są typy klienta z użyciem funkcji qsort, ponieważ ułatwi to przeszukiwanie bazy typów (która również jest posortowana) i rejestrację klienta jako ich odbiorcy. Następnie iteruje się przez bazę typów i jeżeli pierwszy typ z listy klienta został znaleziony, to sprawdza się czy klient już do tej bazy należy. 

  Jeżeli klient należy już do bazy odbiorców tego typu, to nadpisuje się jedynie rodzaj subskrypcji jaką on podał, w przeciwnym wypadku dodaje się go do bazy. Następnie przechodzi się do kolejnego typu klienta i wznawia iterację w bazie typów.

* Funkcja wyswietl_typy_klienta
  
  Wyświetlane są typy i rodzaje subskrypcji klienta.

* Funkcja zaloguj_klienta
  
  Klient jest dopisywany do bazy zalogowani_klienci, przy czym jest sprawdzane, czy klient o podanym id nie jest już zalogowany.

* Funkcja wyswietl_zalogowanych_klientow
  
* Funkcja wyslij_informacje_o_nowym_typie
  
  Jeżeli typ nie został dodany, ponieważ już istniał w bazie, to do procesu go rejestrującego zostaje wysłana wiadomość o niepowodzeniu operacji. W przeciwnym przypadku do wszystkich zalogowanych użytkowników zostaje wysłana informacja o nowym typie.

* Funkcja dodaj_typ
  
  Jeżeli typ jest już w bazie, to nie jest on ponownie dodawany. W przeciwnym przypadku zostaje on dodany do bazy typów i zostaje ona posortowana z użyciem funkcji qsort.

* Funkcja porownaj_typy
  
  Jest to "porównywacz" elementów, który jest używany przez qsort'a. Ta funkcja powoduje, że typy są sortowane względem ich rosnących numerów.

* Funkcja wyswietl_typy
  
  Służy do wyświetlania typów w bazie typów wraz z ich odbiorcami i rodzajami subskrypcji.

* struktury do przechowywania informacji:
  Typ_Subskrypcja, Klient, Wiadomosc, Typ_Wiadomosci


### Opis funkcji w pliku klient.c

Jest to program klienta, który wysyła i odbiera wiadomości od serwera.

* Funkcja main:
  
  Najpierw, jeżeli jeszcze nie została utworzona, tworzona jest kolejka komunikatów serwera. Następnie generowane jest id klienta i jest tworzona jego kolejka komunikatów. Klient proszony jest o wpisanie swojej nazwy.

  Poprzez wywołanie fork tworzony jest nowy proces, w którym (w procesie potomnym) odbierane są wiadomości z użyciem msgrcv (z flagą IPC_NOWAIT) od serwera dotyczące:

    * Typ 3: informacji o zarejestrowanych typach poprzez odebranie struktury typu klient
  
    * Typ 1: odbierania wiadomości z systemu rozgłaszania, co następuje  od razu, gdy wiadomość zostaje wysłana przez serwer do wszystkich subskrybentów poprzez odebranie struktury typu wiadomość
  
  W procesie macierzystym są wysyłane wiadomości do serwera z użyciem msgsnd. Klient ma do wyboru trzy opcje:

    * Opcja 1: rejestracja typu, klient zostaje poproszony o jego podanie i do serwera jest wysyłana struktura klient zawierająca potrzebną informację (Typ 4)
  
    * Opcja 2: rejestracja odbiorcy, klient zostaje poproszony o podanie typu oraz rodzaju jego subskrypcji i do serwera zostaje wysłana struktura klient zawierająca potrzebną informację (Typ 2)

    * Opcja 3: rozgłaszanie nowej wiadomości, klient zostaje poproszony o typ wiadomości oraz jej treść, i do serwera zostaje wysłana struktura klient zawierająca potrzebną informację (Typ 1)
  
* struktury do przechowywania informacji: Typ_Subskrypcja, Klient, Wiadomosc

