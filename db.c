#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <dirent.h>
#include <getopt.h>

#define FALSE 0
#define TRUE 1

#define TABS 3
#define POLETEKSTOWE 20
#define ROZSZERZENIEBAZY ".baz"
#define PLIKTYMCZASOWY "temp.baz"
#define BAZADOMYSLNA "baza"
#define PLIKBAZY "baza.baz"
#define ROZSZERZENIEYAML ".yaml"

#define INTERAKTYWNY 1
#define KONSOLA 0

#define WYBRANY 1
#define WIELE 2
#define WSZYSTKIE 3

#define EKSPORT 1
#define IMPORT 2

typedef struct Wejscie Wejscie;
struct Wejscie {
  int numerKomendy;
  int argumenty;
  int czyBaza;
};

typedef struct Baza Baza;
struct Baza {
  char nazwa[POLETEKSTOWE + 1];
  char nazwaPliku[POLETEKSTOWE + 5];
  long ileRekordow;
};

typedef struct Rekord Rekord;
struct Rekord {
  char imie[POLETEKSTOWE + 1];
  char nazwisko[POLETEKSTOWE + 1];
  char adres[POLETEKSTOWE + 1];
  char telefon[POLETEKSTOWE + 1];
  char email[POLETEKSTOWE + 1];
};

void menuBazy(Baza baza);
void menuGlowne(Baza baza);

void tab() {
  for (int i = 1; i <= TABS; i++) {
    printf("\t");
  }
}

void czyscEkran() {
  printf("\e[1;1H\e[2J");
  fflush(stdout);
}

void pauza() {
  printf("\n");
  tab();
  printf( "Nacisnij klawisz . . ." );
  getch();
}

void zapisano() {
  printf("\n");
  tab();
  printf("-... Zapisano zmiany ...-\n\n");
}

void bladPliku() {
  printf("\n");
  tab();
  printf("-... Blad pliku ...-\n\n");
}

void nieZnaleziono(const char *tytul) {
  printf("\n");
  tab();
  printf("Nie znaleziono %s...\n\n", tytul);
}

void istnieje(const char *tytul) {
  printf("\n");
  tab();
  printf("Juz istnieje %s...\n\n", tytul);
}

void przetransferowano(int ile) {
  printf("\n");
  tab();
  printf("Przetransferowano %d rekordow\n\n", ile);
}

void przetransferowano_info(int ile, const char *typ, const char *z, const char *cel) {
  printf("\n");
  tab();
  printf("Operacja %s z %s do %s.\n", typ, z, cel);
  tab();
  printf("Przetransferowano %d rekordow\n", ile);
  
}

void liniaPozioma() {
  tab(); printf("===============================\n");
}

void liniaPoziomaDluga() {
  tab(); printf("=========================================================================================================\n");
}

void naglowek(const char *tytul, struct Baza baza) {
  czyscEkran();
  liniaPozioma();
  tab(); printf("         %s\n", tytul);
  liniaPozioma();
  tab(); printf("Biezaca Baza: %s\n", strlen(baza.nazwa) == 0 ? "BRAK" : baza.nazwa);
  if (baza.ileRekordow == -1) {
    tab(); printf("Rekordy: %s\n", "BRAK");  
  } else {
    tab(); printf("Rekordy: %ld\n", baza.ileRekordow);
  }
  liniaPozioma();
  fflush(stdout);
}

void tworzRekord(Baza baza, Rekord nowyRekord, char tryb) {
  FILE *plikBazy;
  plikBazy = fopen(baza.nazwaPliku, "a+b");
  if (plikBazy == NULL) {
    bladPliku();
    if (tryb == INTERAKTYWNY) {
      pauza();
    }
  }
  if (tryb == INTERAKTYWNY) {
    naglowek("TWORZ REKORD", baza);
    tab(); printf("Imie: "); fflush(stdin); fgets(nowyRekord.imie, POLETEKSTOWE, stdin);
    if (nowyRekord.imie[strlen(nowyRekord.imie) - 1] == '\n') { nowyRekord.imie[strlen(nowyRekord.imie) - 1] = '\0'; }

    tab(); printf("Nazwisko: "); fflush(stdin); fgets(nowyRekord.nazwisko, POLETEKSTOWE, stdin);
    if (nowyRekord.nazwisko[strlen(nowyRekord.nazwisko) - 1] == '\n') { nowyRekord.nazwisko[strlen(nowyRekord.nazwisko) - 1] = '\0'; }

    tab(); printf("Telefon: "); fflush(stdin); fgets(nowyRekord.telefon, POLETEKSTOWE, stdin);
    if (nowyRekord.telefon[strlen(nowyRekord.telefon) - 1] == '\n') { nowyRekord.telefon[strlen(nowyRekord.telefon) - 1] = '\0'; }

    tab(); printf("Adres: "); fflush(stdin); fgets(nowyRekord.adres, POLETEKSTOWE, stdin);
    if (nowyRekord.adres[strlen(nowyRekord.adres) - 1] == '\n') { nowyRekord.adres[strlen(nowyRekord.adres) - 1] = '\0'; }

    tab(); printf("Email: "); fflush(stdin); fgets(nowyRekord.email, POLETEKSTOWE, stdin);
    if (nowyRekord.email[strlen(nowyRekord.email) - 1] == '\n') { nowyRekord.email[strlen(nowyRekord.email) - 1] = '\0'; }
  }
  fwrite(&nowyRekord, sizeof(struct Rekord), 1, plikBazy);
  fclose(plikBazy);
  zapisano();
  if (tryb == INTERAKTYWNY) {
    pauza();
  }
}

void edytujRekord(Baza baza, Rekord rekordEdytowany, char tryb, char* nazwisko) {
  int opcja;
  int znaleziono = FALSE;
  FILE *plikBazy = fopen(baza.nazwaPliku, "rb");
  if (plikBazy == NULL) {
    bladPliku();
    if (tryb == INTERAKTYWNY) {
      pauza();
    }
    return;
  }
  
  FILE *temp = fopen(PLIKTYMCZASOWY, "wb");
  if (temp == NULL) {
    bladPliku();
    if (tryb == INTERAKTYWNY) {
      pauza();
    }
    return;
  }
  
  Rekord rekordZPliku;

  if (tryb == INTERAKTYWNY) {
    naglowek("EDYTUJ REKORD", baza);
    tab(); printf("Nazwisko: "); fflush(stdin); gets(rekordEdytowany.nazwisko);
    if (rekordEdytowany.nazwisko[strlen(rekordEdytowany.nazwisko) - 1] == '\n') { rekordEdytowany.nazwisko[strlen(rekordEdytowany.nazwisko) - 1] = '\0'; }
  }

  while (!feof(plikBazy)) {
    int rekord = fread(&rekordZPliku, sizeof(struct Rekord), 1, plikBazy);
    if (rekord != 0) {
      if (strcmp(rekordEdytowany.nazwisko, rekordZPliku.nazwisko) == 0) {
        znaleziono = TRUE;
        if (tryb == KONSOLA) {
          strcpy(rekordZPliku.imie, rekordEdytowany.imie);
          strcpy(rekordZPliku.nazwisko, nazwisko);
          strcpy(rekordZPliku.telefon, rekordEdytowany.telefon);
          strcpy(rekordZPliku.adres, rekordEdytowany.adres);
          strcpy(rekordZPliku.email, rekordEdytowany.email);
          fwrite(&rekordZPliku, sizeof(struct Rekord), 1, temp);
          zapisano();
        } else {
          do {
            naglowek("EDYTUJ REKORD", baza);
            tab(); printf("Nazwisko: %s\n", rekordEdytowany.nazwisko);
            tab(); printf("\n");
            tab(); printf("Co edytujemy?\n");
            tab(); printf("1. Imie\n");
            tab(); printf("2. Nazwisko\n");
            tab(); printf("3. Telefon\n");
            tab(); printf("4. Adres\n");
            tab(); printf("5. Email\n");
            tab(); printf("0. Powrot\n");
            liniaPozioma();
            tab(); printf("         WYBOR: "); scanf("%d", &opcja);
            switch (opcja) {
              case 1:
                tab(); printf("Imie: %s\n", rekordZPliku.imie);
                strcpy(rekordZPliku.imie, "");
                tab(); printf("Imie: "); fflush(stdin); fgets(rekordZPliku.imie, POLETEKSTOWE, stdin);
                fflush(stdin);
                if (rekordZPliku.imie[strlen(rekordZPliku.imie) - 1] == '\n') { rekordZPliku.imie[strlen(rekordZPliku.imie) - 1] = '\0'; }
                zapisano();
                break;
              case 2:
                tab(); printf("Nazwisko: %s\n", rekordZPliku.nazwisko);
                strcpy(rekordZPliku.nazwisko, "");
                tab(); printf("Nazwisko: "); fflush(stdin); fgets(rekordZPliku.nazwisko, POLETEKSTOWE, stdin);
                fflush(stdin);
                if (rekordZPliku.nazwisko[strlen(rekordZPliku.nazwisko) - 1] == '\n') { rekordZPliku.nazwisko[strlen(rekordZPliku.nazwisko) - 1] = '\0'; }
                zapisano();
                break;
              case 3:
                tab(); printf("Telefon: %s\n", rekordZPliku.telefon);
                strcpy(rekordZPliku.telefon, "");
                tab(); printf("Telefon: "); fflush(stdin); fgets(rekordZPliku.telefon, POLETEKSTOWE, stdin);
                fflush(stdin);
                if (rekordZPliku.telefon[strlen(rekordZPliku.telefon) - 1] == '\n') { rekordZPliku.telefon[strlen(rekordZPliku.telefon) - 1] = '\0'; }
                zapisano();
                break;
              case 4:
                tab(); printf("Adres: %s\n", rekordZPliku.adres);
                strcpy(rekordZPliku.adres, "");
                tab(); printf("Adres: "); fflush(stdin); fgets(rekordZPliku.adres, POLETEKSTOWE, stdin);
                fflush(stdin);
                if (rekordZPliku.adres[strlen(rekordZPliku.adres) - 1] == '\n') { rekordZPliku.telefon[strlen(rekordZPliku.adres) - 1] = '\0'; }
                zapisano();
                break;
              case 5:
                tab(); printf("Email: %s\n", rekordZPliku.email);
                strcpy(rekordZPliku.email, "");
                tab(); printf("Email: "); fflush(stdin); fgets(rekordZPliku.email, POLETEKSTOWE, stdin);
                fflush(stdin);
                if (rekordZPliku.email[strlen(rekordZPliku.email) - 1] == '\n') { rekordZPliku.email[strlen(rekordZPliku.email) - 1] = '\0'; }
                zapisano();
                break;
              case 0:
                fwrite(&rekordZPliku, sizeof(struct Rekord), 1, temp);
                break;
              default:
                break;
            }
          } while (opcja != 0);
          pauza();
        }
      } else {
        fwrite(&rekordZPliku, sizeof(struct Rekord), 1, temp);
      }
    }
  }
  fclose(plikBazy); fclose(temp);
  remove(baza.nazwaPliku);
  rename(PLIKTYMCZASOWY, baza.nazwaPliku);
  if (znaleziono == FALSE) {
    nieZnaleziono("nazwiska");
    if (tryb == INTERAKTYWNY) {
      pauza();
    }
  }
}

void szukajRekordu(Baza baza, Rekord rekordSzukany, char tryb) {
  int znaleziono = FALSE;
  FILE *plikBazy = fopen(baza.nazwaPliku, "rb");
  if (plikBazy == NULL) {
    bladPliku();
    if (tryb == INTERAKTYWNY) {
      pauza();
    }
    return;
  }

  Rekord rekordZPliku;

  if (tryb == INTERAKTYWNY) {
    naglowek("SZUKAJ REKORDU", baza);
    tab(); printf("Szukane Nazwisko: "); fflush(stdin); gets(rekordSzukany.nazwisko);
    liniaPozioma();
    if (rekordSzukany.nazwisko[strlen(rekordSzukany.nazwisko) - 1] == '\n') { rekordSzukany.nazwisko[strlen(rekordSzukany.nazwisko) - 1] = '\0'; }
  }

  while (!feof(plikBazy)) {
    int rekord = fread(&rekordZPliku, sizeof(struct Rekord), 1, plikBazy);
    if (rekord != 0 && strcmp(rekordSzukany.nazwisko, rekordZPliku.nazwisko) == 0) {
      znaleziono = TRUE;
      tab(); printf("Imie: %s\n", rekordZPliku.imie);
      tab(); printf("Nazwisko: %s\n", rekordZPliku.nazwisko);
      tab(); printf("Telefon: %s\n", rekordZPliku.telefon);
      tab(); printf("Adres: %s\n", rekordZPliku.adres);
      tab(); printf("Email: %s\n", rekordZPliku.email);
      if (tryb == INTERAKTYWNY) {
        pauza();
      }
      break;
    }
  }

  fclose(plikBazy);
  if (znaleziono == FALSE) {
    nieZnaleziono("nazwiska");
    if (tryb == INTERAKTYWNY) {
      pauza();
    }
  }
}

void listaRekordow(Baza baza, char tryb) {
  int znaleziono = FALSE;
  FILE *plikBazy = fopen(baza.nazwaPliku, "rb");
  if (plikBazy == NULL) {
    bladPliku();
    if (tryb == INTERAKTYWNY) {
      pauza();
    }
    return;
  }

  Rekord rekordZPliku;
  if (tryb == INTERAKTYWNY) {
    naglowek("LISTA REKORDOW", baza);
    tab(); printf("\n");
    tab(); printf("%-4s%-20s%-20s%-20s%-20s%-20s\n","#", "IMIE", "NAZWISKO", "TELEFON", "ADRES", "EMAIL");
    liniaPoziomaDluga();
  }
  int indeks = 0;
  while (!feof(plikBazy)) {
    int rekord = fread(&rekordZPliku, sizeof(struct Rekord), 1, plikBazy);
    if (rekord != 0) {
      znaleziono = TRUE;
      indeks++;
      tab(); printf("%-4d%-20s%-20s%-20s%-20s%-20s\n", indeks, rekordZPliku.imie, rekordZPliku.nazwisko, rekordZPliku.telefon, rekordZPliku.adres, rekordZPliku.email);
      fflush(stdout);
    }
  }

  fclose(plikBazy);
  if (znaleziono == FALSE) {
    tab(); printf("Brak danych!\n\n");
  }
  if (tryb == INTERAKTYWNY) {
    pauza();
  }
}

void kasujRekord(Baza baza, Rekord rekordKasowany, char tryb) {
  int znaleziono = FALSE;
  FILE *plikBazy = fopen(baza.nazwaPliku, "rb");
  if (plikBazy == NULL) {
    bladPliku();
    if (tryb == INTERAKTYWNY) {
      pauza();
    }
    return;
  }

  FILE *temp = fopen(PLIKTYMCZASOWY, "wb");
  if (temp == NULL) {
    bladPliku();
    if (tryb == INTERAKTYWNY) {
      pauza();
    }
    return;
  }

  Rekord rekordZPliku;

  if (tryb == INTERAKTYWNY) {
    naglowek("KASUJ REKORD", baza);
    tab(); printf("Nazwisko: "); fflush(stdin); gets(rekordKasowany.nazwisko);
    if (rekordKasowany.nazwisko[strlen(rekordKasowany.nazwisko) - 1] == '\n') { rekordKasowany.nazwisko[strlen(rekordKasowany.nazwisko) - 1] = '\0'; }
  }

  while (!feof(plikBazy)) {
    int rekord = fread(&rekordZPliku, sizeof(struct Rekord), 1, plikBazy);
    if (rekord != 0) {
      if (strcmp(rekordKasowany.nazwisko, rekordZPliku.nazwisko) == 0) {
        znaleziono = TRUE;
      } else {
        fwrite(&rekordZPliku, sizeof(struct Rekord), 1, temp);
      }
    }
  }

  fclose(plikBazy); fclose(temp);
  remove(baza.nazwaPliku);
  rename(PLIKTYMCZASOWY, baza.nazwaPliku);
  remove(PLIKTYMCZASOWY);
  if (znaleziono == FALSE) {
    nieZnaleziono("nazwiska");
  } else {
    zapisano();
  }
  if (tryb == INTERAKTYWNY) {
    pauza();
  }
}

int czyIstniejeBaza(Baza baza) {
  int wynik = FALSE;
  DIR *d;
  struct dirent *dir;
  d = opendir(".");
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      const char *nazwaPliku = dir -> d_name;
      if (strcmp(nazwaPliku, baza.nazwaPliku) == 0) {
        wynik = TRUE;
        closedir(d);
        break;
      }
    }
    closedir(d);
  }
  return wynik;
}

void kopiujBaze(Baza baza) {
  naglowek("KOPIUJ BAZE", baza);
  Baza kopiowana = { "", "", -1 };
  tab(); printf("Nazwa kopiowanej bazy: "); fflush(stdin); gets(kopiowana.nazwa);
  strcpy(kopiowana.nazwaPliku, kopiowana.nazwa);
  strcat(kopiowana.nazwaPliku, ROZSZERZENIEBAZY);
    
  if (czyIstniejeBaza(kopiowana) == TRUE) {
    Baza nowa = { "", "", -1 };
    tab(); printf("Nazwa nowej bazy: "); fflush(stdin); gets(nowa.nazwa);
    strcpy(nowa.nazwaPliku, nowa.nazwa);
    strcat(nowa.nazwaPliku, ROZSZERZENIEBAZY);

    if (czyIstniejeBaza(nowa) == TRUE) {
      istnieje(nowa.nazwa);
      pauza();
    } else {
      FILE *fz = fopen(kopiowana.nazwaPliku, "rb");
      FILE *fd = fopen(nowa.nazwaPliku, "wb");
      if (fz == NULL || fd == NULL) {
        bladPliku();
        return;
      }
      char bufor[4096] = {0};
      int r;
      while ((r = fread(bufor, sizeof(char), sizeof(bufor)/ sizeof(char), fz)) > 0) {
        if (fwrite(bufor, sizeof(char), r, fd) != r) {
          bladPliku();
          fclose(fd);
          fclose(fz);
          return;
        }
      }
      fclose(fd);
      fclose(fz);
      zapisano();
    }
  } else {
    nieZnaleziono(baza.nazwa);
  }
}

void listaBaz(Baza baza) {
  naglowek("LISTA BAZ", baza);
  int licznik = 0;
  DIR *d;
  struct dirent *dir;
  d = opendir(".");
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      const char *nazwaPliku = dir -> d_name;
      int dlugoscNazwy = strlen(nazwaPliku);
      int dlugoscRozszerzenia = strlen(ROZSZERZENIEBAZY);
      if ((dlugoscNazwy >= dlugoscRozszerzenia) && (strcmp(nazwaPliku + (dlugoscNazwy - dlugoscRozszerzenia), ROZSZERZENIEBAZY) == 0)) {
        licznik++;
        tab(); printf("%d. %s\n", licznik, nazwaPliku);
      }
    }
    closedir(d);
  }
  if (licznik == 0) {
    nieZnaleziono("baz");
  }
  pauza();
}

long ileRekordow(Baza baza) {
  FILE *plikBazy = fopen(baza.nazwaPliku, "rb");
  if (plikBazy == NULL) {
    return -1;
  }
  fseek(plikBazy, 0L, SEEK_END);
  long rozmiar = ftell(plikBazy);
  fclose(plikBazy);
  return rozmiar / sizeof(struct Rekord);
}

void kasujBaze(Baza baza) {
  remove(baza.nazwaPliku);
  zapisano();  
}

void transfer(Baza baza1, Baza baza2, Rekord rekordEksportowany, char tryb, char kierunek) {
  FILE *plikBazyEksportu;
  FILE *plikBazyImportu;
  
  if (kierunek == EKSPORT) {
    plikBazyEksportu = fopen(baza1.nazwaPliku, "rb");
    plikBazyImportu = fopen(baza2.nazwaPliku, "a+b");
  } else {
    plikBazyEksportu = fopen(baza2.nazwaPliku, "rb");
    plikBazyImportu = fopen(baza1.nazwaPliku, "a+b");
  }
  
  if (plikBazyEksportu == NULL || plikBazyImportu == NULL) {
    bladPliku();
    return;
  }

  Rekord rekordZPliku;
  int ile = 0;

  while (!feof(plikBazyEksportu)) {
    int rekord = fread(&rekordZPliku, sizeof(struct Rekord), 1, plikBazyEksportu);
    if (rekord != 0) {
      switch (tryb) {
        case WYBRANY:
          if (rekord != 0 && strcmp(rekordEksportowany.nazwisko, rekordZPliku.nazwisko) == 0) {
            ile++;
            fwrite(&rekordZPliku, sizeof(struct Rekord), 1, plikBazyImportu);
          }
          break;
        case WIELE:
          if (strstr(rekordZPliku.nazwisko, rekordEksportowany.nazwisko) != NULL) {
            ile++;
            fwrite(&rekordZPliku, sizeof(struct Rekord), 1, plikBazyImportu);
          }
          break;
        case WSZYSTKIE:
          ile++;
          fwrite(&rekordZPliku, sizeof(struct Rekord), 1, plikBazyImportu);
          break;
      }
    }
  }

  fclose(plikBazyEksportu);
  fclose(plikBazyImportu);
  char *k = "EKSPORT";
  char *be = baza1.nazwa;
  char *bi = baza2.nazwa;
  if (kierunek == IMPORT) {
    k = "IMPORT";
    be = baza2.nazwa;
    bi = baza1.nazwa;
  }
  przetransferowano_info(ile, k, be, bi);
}

void import_text(Baza baza, Rekord rekordImportowany, char *nazwaPliku, char tryb) {
  FILE *plikBazy = fopen(baza.nazwaPliku, "a+b");
  if (plikBazy == NULL) {
    bladPliku();
    return;
  }

  FILE *plikImportu = fopen(nazwaPliku, "r");
  if (plikImportu == NULL) {
    bladPliku();
    return;
  }

  Rekord rekordZPliku;
  char klucz[POLETEKSTOWE + 1];
  char wartosc[POLETEKSTOWE + 1];
  int ile = 0;

  int licznik = 0;
  while(fscanf(plikImportu, "%s %s \n", klucz, wartosc) != EOF) {
    int mod = licznik % 6;
    switch(mod) {
      case 0:
        break;
      case 1:
        strcpy(rekordZPliku.imie, wartosc);
        break;
      case 2:
        strcpy(rekordZPliku.nazwisko, wartosc);
        break;
      case 3:
        strcpy(rekordZPliku.telefon, wartosc);
        break;
      case 4:
        strcpy(rekordZPliku.adres, wartosc);
        break;
      case 5:
        strcpy(rekordZPliku.email, wartosc);
        switch (tryb) {
          case WYBRANY:
            if (strcmp(rekordImportowany.nazwisko, rekordZPliku.nazwisko) == 0) {
              ile++;
              fwrite(&rekordZPliku, sizeof(struct Rekord), 1, plikBazy);
            }
            break;
          case WIELE:
            if (strstr(rekordImportowany.nazwisko, rekordZPliku.nazwisko) != NULL) {
              ile++;
              fwrite(&rekordZPliku, sizeof(struct Rekord), 1, plikBazy);
            }
            break;
          case WSZYSTKIE:
            ile++;
            fwrite(&rekordZPliku, sizeof(struct Rekord), 1, plikBazy);
            break;  
        }
        break;
    }
    licznik++;
  }

  fclose(plikImportu);
  fclose(plikBazy);
  przetransferowano(ile);
}

void eksport_text(Baza baza, Rekord rekordEksportowany, char *nazwaPliku, char tryb) {
  FILE *plikBazy = fopen(baza.nazwaPliku, "rb");
  if (plikBazy == NULL) {
    bladPliku();
    return;
  }

  FILE *plikEksportu = fopen(nazwaPliku, "w");
  if (plikEksportu == NULL) {
    bladPliku();
    return;
  }

  Rekord rekordZPliku;
  int ile = 0;

  while (!feof(plikBazy)) {
    int rekord = fread(&rekordZPliku, sizeof(struct Rekord), 1, plikBazy);
    if (rekord != 0) {
      switch (tryb) {
        case WYBRANY:
          if (rekord != 0 && strcmp(rekordEksportowany.nazwisko, rekordZPliku.nazwisko) == 0) {
            ile++;
            fprintf(plikEksportu, "rekord: %s\n", rekordZPliku.nazwisko);
            fprintf(plikEksportu, "  imie: %s\n", rekordZPliku.imie);
            fprintf(plikEksportu, "  nazwisko: %s\n", rekordZPliku.nazwisko);
            fprintf(plikEksportu, "  telefon: %s\n", rekordZPliku.telefon);
            fprintf(plikEksportu, "  adres: %s\n", rekordZPliku.adres);
            fprintf(plikEksportu, "  email: %s\n", rekordZPliku.email);
          }
          break;
        case WIELE:
          if (strstr(rekordZPliku.nazwisko, rekordEksportowany.nazwisko) != NULL) {
            ile++;
            fprintf(plikEksportu, "rekord: %s\n", rekordZPliku.nazwisko);
            fprintf(plikEksportu, "  imie: %s\n", rekordZPliku.imie);
            fprintf(plikEksportu, "  nazwisko: %s\n", rekordZPliku.nazwisko);
            fprintf(plikEksportu, "  telefon: %s\n", rekordZPliku.telefon);
            fprintf(plikEksportu, "  adres: %s\n", rekordZPliku.adres);
            fprintf(plikEksportu, "  email: %s\n", rekordZPliku.email);
          }
          break;
        case WSZYSTKIE:
          ile++;
            fprintf(plikEksportu, "rekord: %s\n", rekordZPliku.nazwisko);
            fprintf(plikEksportu, "  imie: %s\n", rekordZPliku.imie);
            fprintf(plikEksportu, "  nazwisko: %s\n", rekordZPliku.nazwisko);
            fprintf(plikEksportu, "  telefon: %s\n", rekordZPliku.telefon);
            fprintf(plikEksportu, "  adres: %s\n", rekordZPliku.adres);
            fprintf(plikEksportu, "  email: %s\n", rekordZPliku.email);
          break;
      }
    }
  }

  fclose(plikEksportu);
  fclose(plikBazy);
  przetransferowano(ile);
}

void opcjeMenuGlownego() {
  tab(); printf("1. Tworz baze\n");
  tab(); printf("2. Otworz baze\n");
  tab(); printf("3. Kopiuj baze\n");
  tab(); printf("4. Lista baz\n");
  tab(); printf("5. Kasuj baze\n");
  tab(); printf("0. Wyjscie z programu\n");
  liniaPozioma();
  tab(); printf("             WYBOR: ");
}

void menuGlowne(Baza baza) {
  int opcja;
  do {
    naglowek("MENU GLOWNE", baza);
    opcjeMenuGlownego();
    int wejscie = scanf("%d", &opcja);
    fflush(stdin);
    if (wejscie == 1) {
      switch (opcja) {
        case 1:
          tab(); printf("Nazwa bazy: "); fflush(stdin); gets(baza.nazwa);
          strcpy(baza.nazwaPliku, baza.nazwa);
          strcat(baza.nazwaPliku, ROZSZERZENIEBAZY);
          if (czyIstniejeBaza(baza) == TRUE) {
            istnieje(baza.nazwa);
            pauza();
            strcpy(baza.nazwa, "");
            strcpy(baza.nazwaPliku, "");
          } else {
            FILE *plikBazy = fopen(baza.nazwaPliku, "wb");
            if (plikBazy == NULL) {
              bladPliku();
              pauza();
              return;
            }
            fclose(plikBazy);
            menuBazy(baza);
            break;
          }
          break;
        case 2:
          tab(); printf("Nazwa bazy: "); fflush(stdin); gets(baza.nazwa);
          strcpy(baza.nazwaPliku, baza.nazwa);
          strcat(baza.nazwaPliku, ROZSZERZENIEBAZY);
          if (czyIstniejeBaza(baza) == TRUE) {
            menuBazy(baza);
            break;
          } else {
            nieZnaleziono(baza.nazwa);
            pauza();
            strcpy(baza.nazwa, "");
            strcpy(baza.nazwaPliku, "");
          }
          break;
        case 3:
          kopiujBaze(baza);
          break;
        case 4:
          listaBaz(baza);
          break;
        case 5:
          tab(); printf("Nazwa bazy: "); fflush(stdin); gets(baza.nazwa);
          strcpy(baza.nazwaPliku, baza.nazwa);
          strcat(baza.nazwaPliku, ROZSZERZENIEBAZY);
          if (czyIstniejeBaza(baza) == TRUE) {
            kasujBaze(baza);
            strcpy(baza.nazwa, "");
            strcpy(baza.nazwaPliku, "");
            break;
          } else {
            nieZnaleziono(baza.nazwa);
            pauza();
          }
          break;
        case 0:
          exit(0);
          break;
        default:
          break;
      }
    }
  } while (opcja != 0);
}

void opcjeMenuBazy() {
  tab(); printf("1. Tworz rekord\n");
  tab(); printf("2. Edytuj rekord\n");
  tab(); printf("3. Szukaj rekordu\n");
  tab(); printf("4. Lista rekordow\n");
  tab(); printf("5. Kasuj rekord\n");
  tab(); printf("0. Wyjscie z bazy\n");
}

void menuBazy(Baza baza) {
  int opcja;
  do {
    baza.ileRekordow = ileRekordow(baza);
    naglowek("MENU BAZY", baza);
    opcjeMenuBazy();
    liniaPozioma();
    tab(); printf("             WYBOR: ");
    int wejscie = scanf("%d", &opcja);
    if (wejscie == 1) {
      switch (opcja) {
        case 1:
          tworzRekord(baza, (Rekord){ "", "", "", "", ""}, INTERAKTYWNY);
          break;
        case 2:
          edytujRekord(baza, (Rekord){ "", "", "", "", ""}, INTERAKTYWNY, "");
          break;
        case 3:
          szukajRekordu(baza, (Rekord){ "", "", "", "", ""}, INTERAKTYWNY);
          break;
        case 4:
          listaRekordow(baza, INTERAKTYWNY);
          break;
        case 5:
          kasujRekord(baza, (Rekord){ "", "", "", "", ""}, INTERAKTYWNY);
          break;
        case 0:
          menuGlowne((Baza){ "", "", -1 });
          break;
        default:
          break;
      }
    }
  } while (opcja != 0);
}

void uzycie(char *argument) {
  fprintf(stderr, "Uzycie: %s --komenda dodaj|pokaz|edytuj|usun|lista|eksport|import|eksportw|importw|eksportc|importc\n", argument);
  fprintf(stderr, "Argumenty: %s dodaj <imie> <nazwisko> <telefon> <adres> <email>\n", argument);
  fprintf(stderr, "Argumenty: %s edytuj <szukaneNazwisko> <imie> <nazwisko> <telefon> <adres> <email>\n", argument);
  fprintf(stderr, "Argumenty: %s pokaz|usun|eksport|import <nazwisko>\n", argument);
  fprintf(stderr, "Argumenty: %s eksport <nazwisko> <baza_exp> <baza_imp>\n", argument);
  fprintf(stderr, "Argumenty: %s eksportw <czescNazwiska> <baza_exp> <baza_imp>\n", argument);
  fprintf(stderr, "Argumenty: %s eksportc <baza_exp> <baza_imp>\n", argument);
  fprintf(stderr, "Argumenty: %s import <nazwisko> <baza_imp> <baza_exp>\n", argument);
  fprintf(stderr, "Argumenty: %s importw <czescNazwiska> <baza_imp> <baza_exp>\n", argument);
  fprintf(stderr, "Argumenty: %s importc <baza_imp> <baza_exp>\n", argument);
  fprintf(stderr, "Argumenty: %s eksport_text|import_text <nazwisko> <plik>\n", argument);
  fprintf(stderr, "Argumenty: %s eksport_text_w|import_text_w <czescNazwiska> <plik>\n", argument);
  fprintf(stderr, "Argumenty: %s eksport_text_c|import_text_c <plik>\n", argument);
  fprintf(stderr, "Bez argumentow: %s lista\n", argument);
}

int numerKomendy(const char *argument) {
  int numer = -1;
  if (strcmp(argument, "dodaj") == 0) {
    numer = 0;
  } else if (strcmp(argument, "pokaz") == 0) {
    numer = 1;
  } else if (strcmp(argument, "edytuj") == 0) {
    numer = 2;
  } else if (strcmp(argument, "usun") == 0) {
    numer = 3;
  } else if (strcmp(argument, "eksport") == 0) {
    numer = 4;
  } else if (strcmp(argument, "import") == 0) {
    numer = 5;
  } else if (strcmp(argument, "lista") == 0) {
    numer = 6;
  } else if (strcmp(argument, "eksportw") == 0) {
    numer = 7;
  } else if (strcmp(argument, "importw") == 0) {
    numer = 8;
  } else if (strcmp(argument, "eksportc") == 0) {
    numer = 9;
  } else if (strcmp(argument, "importc") == 0) {
    numer = 10;
  } else if (strcmp(argument, "eksport_text") == 0) {
    numer = 11;
  } else if (strcmp(argument, "eksport_text_w") == 0) {
    numer = 12;
  } else if (strcmp(argument, "eksport_text_c") == 0) {
    numer = 13;
  } else if (strcmp(argument, "import_text") == 0) {
    numer = 14;
  } else if (strcmp(argument, "import_text_w") == 0) {
    numer = 15;
  } else if (strcmp(argument, "import_text_c") == 0) {
    numer = 16;
  }
  return numer;
}

int ileArgumentow(int numerKomendy) {
  int argumenty = 1;
  if (numerKomendy == 0) {
    argumenty = 5;
  } else if (numerKomendy == 2) {
    argumenty = 6;
  } else if (numerKomendy == 4 || numerKomendy == 5 || numerKomendy == 7 || numerKomendy == 8 ) {
    argumenty = 3;
  } else if (numerKomendy == 6) {
    argumenty = 0;
  } else if (numerKomendy == 9 || numerKomendy == 10 || numerKomendy == 11 || numerKomendy == 12 || numerKomendy == 14 || numerKomendy == 15 ) {
    argumenty = 2;
  }
  return argumenty;
}

void wykonajKomende(int numerKomendy, char *nazwaBazy, int daneBazy, char *argv[]) {

  Baza baza = (Baza){ BAZADOMYSLNA, PLIKBAZY, -1 };
  strcpy(baza.nazwa, nazwaBazy);
  strcpy(baza.nazwaPliku, baza.nazwa);
  strcat(baza.nazwaPliku, ROZSZERZENIEBAZY);
  Rekord rekord = (Rekord){ "", "", "", "", "" };
  Baza bazaE = (Baza){ "", "", -1 };
  Baza bazaI = (Baza){ "", "", -1 };
  char nazwaPliku[21];
  int pierwszyArg = 3;

  if (daneBazy == TRUE) {
    pierwszyArg = pierwszyArg + 2;
  }
    
  switch(numerKomendy) {
    case 0:
      strcpy(rekord.imie, argv[pierwszyArg]);
      strcpy(rekord.nazwisko, argv[pierwszyArg + 1]);
      strcpy(rekord.telefon, argv[pierwszyArg + 2]);
      strcpy(rekord.adres, argv[pierwszyArg + 3]);
      strcpy(rekord.email, argv[pierwszyArg + 4]);
      tworzRekord(baza, rekord, KONSOLA);
      break;
    case 1:
      strcpy(rekord.nazwisko, argv[pierwszyArg]);
      szukajRekordu(baza, rekord, KONSOLA);
      break;
    case 2:
      strcpy(rekord.imie, argv[pierwszyArg + 1]);
      strcpy(rekord.nazwisko, argv[pierwszyArg]);
      strcpy(rekord.telefon, argv[pierwszyArg + 3]);
      strcpy(rekord.adres, argv[pierwszyArg + 4]);
      strcpy(rekord.email, argv[pierwszyArg + 5]);
      edytujRekord(baza, rekord, KONSOLA, argv[pierwszyArg + 2]);
      break;
    case 3:
      strcpy(rekord.nazwisko, argv[pierwszyArg]);
      kasujRekord(baza, rekord, KONSOLA);
      break;
    case 4:
      strcpy(rekord.nazwisko, argv[pierwszyArg]);
      strcpy(bazaE.nazwa, argv[pierwszyArg + 1]);
      strcpy(bazaE.nazwaPliku, bazaE.nazwa);
      strcat(bazaE.nazwaPliku, ROZSZERZENIEBAZY);
      strcpy(bazaI.nazwa, argv[pierwszyArg + 2]);
      strcpy(bazaI.nazwaPliku, bazaI.nazwa);
      strcat(bazaI.nazwaPliku, ROZSZERZENIEBAZY);
      transfer(bazaE, bazaI, rekord, WYBRANY, EKSPORT);
      break;
    case 5:
      strcpy(rekord.nazwisko, argv[pierwszyArg]);
      strcpy(bazaE.nazwa, argv[pierwszyArg + 1]);
      strcpy(bazaE.nazwaPliku, bazaE.nazwa);
      strcat(bazaE.nazwaPliku, ROZSZERZENIEBAZY);
      strcpy(bazaI.nazwa, argv[pierwszyArg + 2]);
      strcpy(bazaI.nazwaPliku, bazaI.nazwa);
      strcat(bazaI.nazwaPliku, ROZSZERZENIEBAZY);
      transfer(bazaE, bazaI, rekord, WYBRANY, IMPORT);
      break;
    case 6:
      listaRekordow(baza, KONSOLA);
      break;  
    case 7:
      strcpy(rekord.nazwisko, argv[pierwszyArg]);
      strcpy(bazaE.nazwa, argv[pierwszyArg + 1]);
      strcpy(bazaE.nazwaPliku, bazaE.nazwa);
      strcat(bazaE.nazwaPliku, ROZSZERZENIEBAZY);
      strcpy(bazaI.nazwa, argv[pierwszyArg + 2]);
      strcpy(bazaI.nazwaPliku, bazaI.nazwa);
      strcat(bazaI.nazwaPliku, ROZSZERZENIEBAZY);
      transfer(bazaE, bazaI, rekord, WIELE, EKSPORT);
      break;
    case 8:
      strcpy(rekord.nazwisko, argv[pierwszyArg]);
      strcpy(bazaE.nazwa, argv[pierwszyArg + 1]);
      strcpy(bazaE.nazwaPliku, bazaE.nazwa);
      strcat(bazaE.nazwaPliku, ROZSZERZENIEBAZY);
      strcpy(bazaI.nazwa, argv[pierwszyArg + 2]);
      strcpy(bazaI.nazwaPliku, bazaI.nazwa);
      strcat(bazaI.nazwaPliku, ROZSZERZENIEBAZY);
      transfer(bazaE, bazaI, rekord, WIELE, IMPORT);
      break;
    case 9:
      strcpy(bazaE.nazwa, argv[pierwszyArg]);
      strcpy(bazaE.nazwaPliku, bazaE.nazwa);
      strcat(bazaE.nazwaPliku, ROZSZERZENIEBAZY);
      strcpy(bazaI.nazwa, argv[pierwszyArg + 1]);
      strcpy(bazaI.nazwaPliku, bazaI.nazwa);
      strcat(bazaI.nazwaPliku, ROZSZERZENIEBAZY);
      transfer(bazaE, bazaI, rekord, WSZYSTKIE, EKSPORT);
      break;
    case 10:
      strcpy(bazaE.nazwa, argv[pierwszyArg]);
      strcpy(bazaE.nazwaPliku, bazaE.nazwa);
      strcat(bazaE.nazwaPliku, ROZSZERZENIEBAZY);
      strcpy(bazaI.nazwa, argv[pierwszyArg + 1]);
      strcpy(bazaI.nazwaPliku, bazaI.nazwa);
      strcat(bazaI.nazwaPliku, ROZSZERZENIEBAZY);
      transfer(bazaE, bazaI, rekord, WSZYSTKIE, IMPORT);
      break;
    case 11:
      strcpy(rekord.nazwisko, argv[pierwszyArg]);
      strcpy(nazwaPliku, argv[pierwszyArg + 1]);
      strcat(nazwaPliku, ROZSZERZENIEYAML);
      eksport_text(baza, rekord, nazwaPliku, WYBRANY);
      break;
    case 12:
      strcpy(rekord.nazwisko, argv[pierwszyArg]);
      strcpy(nazwaPliku, argv[pierwszyArg + 1]);
      strcat(nazwaPliku, ROZSZERZENIEYAML);
      eksport_text(baza, rekord, nazwaPliku, WIELE);
      break;
    case 13:
      strcpy(nazwaPliku, argv[pierwszyArg]);
      strcat(nazwaPliku, ROZSZERZENIEYAML);
      eksport_text(baza, rekord, nazwaPliku, WSZYSTKIE);
      break;
    case 14:
      strcpy(rekord.nazwisko, argv[pierwszyArg]);
      strcpy(nazwaPliku, argv[pierwszyArg + 1]);
      strcat(nazwaPliku, ROZSZERZENIEYAML);
      import_text(baza, rekord, nazwaPliku, WYBRANY);
      break;
    case 15:
      strcpy(rekord.nazwisko, argv[pierwszyArg]);
      strcpy(nazwaPliku, argv[pierwszyArg + 1]);
      strcat(nazwaPliku, ROZSZERZENIEYAML);
      import_text(baza, rekord, nazwaPliku, WIELE);
      break;
    case 16:
      strcpy(nazwaPliku, argv[pierwszyArg]);
      strcat(nazwaPliku, ROZSZERZENIEYAML);
      import_text(baza, rekord, nazwaPliku, WSZYSTKIE);
      break;
  }
}

int poprawnaLiczbaArgumentow(int argumenty, int daneBazy) {
  int liczbaArgumentow = argumenty + 3;
    if (daneBazy == TRUE) {
      liczbaArgumentow = liczbaArgumentow + 2;
    }
  return liczbaArgumentow;
}

int main(int argc, char *argv[]) {
  int opcja;
  int indeksOpcji = 0;
  struct option opcje[] = {
    { "komenda", required_argument, 0, 'k'},
    { "baza", required_argument, 0, 'b'},
    { 0, 0, 0, 0 }
  };
  if (argc == 1) { 
    menuGlowne((Baza){ "", "", -1 });
  } else {
    if (argc == 2) {
      uzycie(argv[0]);
      return 1;
    }
    int argumenty = 1;
    int komenda = -1;
    char *nazwaBazy = BAZADOMYSLNA;
    int daneBazy = FALSE;
    while ((opcja = getopt_long(argc, argv, "k:b:", opcje, &indeksOpcji)) != -1) {
      switch(opcja) {
        case 'k':
          komenda = numerKomendy(optarg);
          if (komenda != -1) {
            argumenty = ileArgumentow(komenda);
          } else {
            uzycie(argv[0]);
            return 1;
          }
          break;
        case 'b':
          nazwaBazy = optarg;
          daneBazy = TRUE;
          break;
        case '?':  
          uzycie(argv[0]);
          return 1;
      }
    }

    int liczbaArgumentow = poprawnaLiczbaArgumentow(argumenty, daneBazy);
    if (argc != liczbaArgumentow) {
      uzycie(argv[0]);
      return 1;
    }
    wykonajKomende(komenda, nazwaBazy, daneBazy, argv);
  }
}
