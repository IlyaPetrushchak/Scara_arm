# INSTRUKCJA OBSŁUGI ROBOTA SCARA

## 1. Przeznaczenie

Robot jest edukacyjnym manipulatorem SCARA przeznaczonym do nauki programowania, automatyki, robotyki, kinematyki i pracy metodą TEACH. Sterowanie odbywa się z wykorzystaniem ESP32, dedykowanej płytki PCB, silników krokowych, dwóch serwomechanizmów i kontrolera Bluetooth.

Urządzenie ma charakter dydaktyczny i eksperymentalny. Nie jest certyfikowanym robotem przemysłowym.

## 2. Elementy sterowania

Robot posiada:

- kontroler Bluetooth,
- przycisk `MODE` na PCB,
- przycisk `TEACH` na PCB,
- E-STOP,
- diodę Bluetooth,
- diodę STATUS,
- diodę ERROR,
- wyświetlacz OLED 128×64.

## 3. Pozycja startowa

Przed włączeniem zasilania lub wykonaniem resetu należy ręcznie ustawić:

- ARM1: `0°`,
- ARM2: `0°`.

Oś Z nie musi być wcześniej ustawiona dokładnie na zero. Jej punkt HOME ustala się po uruchomieniu z wykorzystaniem pada.

Pozycje startowe serw:

- serwo 1 – obrót narzędzia: `90°`,
- serwo 2 – chwytak: `90°`.

Zakres serwa 2 jest ograniczony programowo do `90–180°`.

## 4. Uruchomienie robota

1. Sprawdź, czy w zasięgu robota nie znajdują się osoby ani przedmioty mogące wejść w kolizję z ramieniem.
2. Ustaw ręcznie ARM1 i ARM2 na `0°`.
3. Zwolnij E-STOP.
4. Włącz zasilanie lub zresetuj ESP32.
5. OLED przez około 5 s wyświetli logo startowe.
6. Następnie pojawi się ekran `SET Z HOME`.
7. Połącz pad Bluetooth ze sterownikiem.
8. Nie dotykaj analogów ani przycisków przez co najmniej 300 ms, aż pad zostanie uzbrojony.
9. Wykonaj procedurę HOME osi Z opisaną w następnym rozdziale.

Do momentu zatwierdzenia HOME Z normalne sterowanie, `MODE` i `TEACH` pozostają zablokowane.

## 5. Ustawianie HOME osi Z

Podczas startowej procedury HOME Z:

- prawy analog Y steruje wyłącznie osią Z,
- maksymalna prędkość ruchu wynosi około `1.5 mm/s`,
- normalne programowe ograniczenie `0–300 mm` nie jest jeszcze używane, ponieważ sterownik nie zna pozycji absolutnej.

Procedura:

1. Poruszaj osią Z prawym analogiem Y.
2. Ustaw mechanicznie pozycję, która ma odpowiadać `Z = 0`.
3. Puść analog.
4. Poczekaj, aż oś Z całkowicie się zatrzyma.
5. Naciśnij jednocześnie `A+B`.
6. Sterownik przypisze aktualnej pozycji wartość `Z = 0.00 mm`.
7. Puść oba przyciski.
8. Pozostaw pad neutralnie przez około 300 ms, aby ponownie uzbroić normalne sterowanie MANUAL.

Po zatwierdzeniu Z HOME nie należy ręcznie przestawiać osi ani ramion. Sterownik nie ma enkoderów i nie wykryje takiej zmiany.

## 6. Tryb MANUAL – JOINT

Po uruchomieniu robot pracuje w trybie MANUAL. `START` na padzie przełącza między `JOINT` i `TOOL`.

W trybie JOINT sterowanie jest bezpośrednie:

| Element pada | Funkcja |
|---|---|
| Lewy analog X | ARM1 |
| Lewy analog Y | ARM2 |
| Prawy analog Y | Oś Z |
| Prawy analog X | Obrót narzędzia – serwo 1 |
| A | Zwiększenie kąta serwa 2 |
| B | Zmniejszenie kąta serwa 2 |
| START | Przełączenie JOINT / TOOL |

Programowe zakresy osi:

- ARM1: `−105° ... +105°`,
- ARM2: `−125° ... +125°`,
- Z: `0 ... 300 mm`,
- serwo 1: `0 ... 180°`,
- serwo 2: `90 ... 180°`.

## 7. Tryb MANUAL – TOOL

W trybie TOOL operator steruje pozycją końcówki TCP w płaszczyźnie XY, a sterownik sam przelicza wymagane kąty ARM1 i ARM2 przy pomocy kinematyki odwrotnej.

| Element pada | Funkcja |
|---|---|
| Lewy analog X | TCP w osi X |
| Lewy analog Y | TCP w osi Y |
| Prawy analog Y | Oś Z |
| Prawy analog X | Obrót narzędzia |
| A / B | Zmiana kąta serwa 2 |
| START | Powrót do JOINT |

Jeżeli żądany punkt XY znajduje się poza przestrzenią roboczą lub rozwiązanie kinematyki jest niedozwolone, ruch zostanie odrzucony i może zapalić się dioda ERROR.

## 8. Zapisywanie punktów TEACH

Punkty można zapisywać tylko w MANUAL i przy zatrzymanych osiach.

Krótko naciśnij przycisk `TEACH` na PCB, aby zapisać aktualny punkt.

Zapisywane są:

- pozycja ARM1,
- pozycja ARM2,
- pozycja Z,
- pozycja serwa 1,
- pozycja serwa 2,
- czas postoju punktu.

Sterownik przechowuje maksymalnie 20 punktów. Punkty są zapisywane w pamięci NVS i pozostają po odłączeniu zasilania.

Aby skasować cały program, przytrzymaj `TEACH` przez co najmniej `1.8 s`.

## 9. Tryb AUTO

Do AUTO można przejść tylko przy zatrzymanych osiach.

1. Zatrzymaj robota.
2. Naciśnij `MODE` na PCB.
3. Dioda STATUS powinna wskazać tryb AUTO.
4. Naciśnij `START` na padzie, aby rozpocząć program.

`START` w AUTO działa kolejno jako:

- start,
- pauza,
- wznowienie.

Program odtwarza zapisane punkty metodą PTP i domyślnie pracuje w pętli.

## 10. Regulacja prędkości AUTO

Prędkość AUTO można zmieniać D-padem:

- D-pad góra: `+10%`,
- D-pad dół: `−10%`,
- D-pad lewo: powrót do `100%`.

Dostępny zakres to `50–250%`.

Bazowe prędkości wynoszą:

- ARM1: `20°/s`,
- ARM2: `20°/s`,
- Z: `3.0 mm/s`.

Zmiana procentu zaczyna obowiązywać od kolejnego punktu PTP i skaluje wszystkie trzy główne osie.

## 11. Zatrzymanie programu AUTO

Naciśnięcie `START` podczas ruchu przełącza AUTO w PAUSE.

Po ponownym naciśnięciu `START` program jest wznawiany od bieżącego punktu.

Rozłączenie pada podczas AUTO również powoduje pauzę programu.

## 12. E-STOP

E-STOP należy nacisnąć natychmiast w przypadku:

- zagrożenia kolizją,
- niekontrolowanego ruchu,
- zablokowania mechaniki,
- problemu z napędem,
- zagrożenia dla operatora lub urządzenia.

Po aktywacji E-STOP firmware:

- zatrzymuje generowanie kroków,
- blokuje ruch,
- ustawia AUTO w stan FAULT,
- zapala sygnalizację błędu.

Po E-STOP wymagany jest restart sterownika. Po restarcie należy ponownie ustawić ARM1/ARM2 na `0°` i wykonać HOME Z.

## 13. Sygnalizacja

### LED Bluetooth

- światło ciągłe – pad połączony,
- miganie – brak połączenia z padem.

### LED STATUS

- świeci w trybie AUTO,
- zgaszona w MANUAL.

### LED ERROR

Może sygnalizować m.in.:

- timeout pada,
- niedozwolony punkt TOOL / błąd IK,
- E-STOP,
- FAULT AUTO.

### OLED

OLED pokazuje zależnie od stanu m.in.:

- ekran startowy,
- procedurę `SET Z HOME`,
- stan połączenia Bluetooth,
- uzbrojenie pada,
- tryb MANUAL/AUTO,
- JOINT/TOOL,
- położenie osi,
- liczbę punktów TEACH,
- stan programu AUTO,
- procent prędkości AUTO.

## 14. Utrata kroków i pozycja robota

Pozycja robota jest obliczana z liczników kroków silników. Sterownik nie otrzymuje sprzężenia zwrotnego z enkoderów.

Jeżeli silnik zgubi kroki:

- pozycja programowa przestaje odpowiadać rzeczywistej,
- zapis kolejnych punktów TEACH może być błędny,
- AUTO może pojechać do innego miejsca niż oczekiwane.

W takiej sytuacji należy zatrzymać robota i wykonać restart oraz ponowne ustawienie HOME.

## 15. Zasady bezpiecznej pracy

- Nie wkładaj rąk w przestrzeń roboczą podczas ruchu.
- Nie zmieniaj ręcznie położenia osi po ustaleniu HOME.
- Nie zapisuj punktu TEACH podczas ruchu.
- Nie przełączaj MANUAL/AUTO podczas ruchu.
- Pierwsze odtworzenie nowego programu wykonuj z obniżoną prędkością AUTO.
- Obserwuj przewody, paski, śrubę Z i chwytak pod kątem kolizji.
- Po każdej zmianie mechanicznej lub zmianie przełożenia zweryfikuj parametry firmware.
- Nie odłączaj silnika krokowego od sterownika przy załączonym zasilaniu.
- E-STOP powinien być zawsze łatwo dostępny.

## 16. Zalecana procedura pierwszego uruchomienia programu

1. Uruchom robota i wykonaj HOME Z.
2. Pozostań w MANUAL.
3. Sprawdź każdy kierunek ruchu osi przy małym wychyleniu analogu.
4. Sprawdź zakres serw.
5. Zapisz 2–3 bezpieczne punkty TEACH z dużym zapasem od przeszkód.
6. Przejdź do AUTO.
7. Ustaw prędkość na `50%`.
8. Uruchom program i trzymaj rękę w pobliżu E-STOP.
9. Po potwierdzeniu poprawnego przebiegu stopniowo zwiększaj prędkość.

## 17. Wyłączenie robota

1. Zatrzymaj ruch.
2. Jeżeli działa AUTO, przełącz go w PAUSE lub wróć do MANUAL.
3. Ustaw mechanikę w bezpiecznej pozycji.
4. Wyłącz zasilanie.

Po kolejnym włączeniu zawsze należy ponownie wykonać procedurę HOME osi Z.
