# DOKUMENTACJA TECHNICZNA
## ROBOTYCZNE RAMIĘ SCARA
### Projekt „Robotyka bez granic”

**Typ urządzenia:** edukacyjne ramię robotyczne SCARA 3 DOF + chwytak 2 DOF  
**Sterownik:** ESP32 D1 Mini / WeMos D1 Mini ESP32  
**Sterowanie:** kontroler Bluetooth + przyciski na płytce  
**Oprogramowanie:** C++ / Arduino Framework / PlatformIO  
**Repozytorium:** `Attiny2313/SCARA-Robot-Controler-BT`  
**Gałąź odpowiadająca niniejszemu opisowi:** `dev`  
**Bazowy commit firmware/config:** `3dcf2272da6c95ba952772bd750d242ef21eb560`  

> Dokument opisuje rzeczywistą wersję projektu rozwijaną w repozytorium. Dane pochodzące z projektu bazowego PyBot zostały oddzielone od parametrów wynikających z aktualnej konstrukcji i programu sterującego.

---

# 1. CEL PROJEKTU

Celem projektu jest wykonanie, uruchomienie i zaprogramowanie edukacyjnego robota typu SCARA (Selective Compliance Assembly Robot Arm), przeznaczonego do nauki zagadnień z zakresu mechaniki, elektroniki, automatyki, robotyki, programowania oraz technologii druku 3D.

Robot umożliwia:

- ręczne sterowanie poszczególnymi osiami,
- sterowanie położeniem końcówki roboczej w układzie kartezjańskim,
- sterowanie chwytakiem,
- zapisywanie punktów metodą TEACH,
- automatyczne odtwarzanie zapisanej sekwencji ruchów,
- realizację zadań typu Pick and Place,
- dalszą rozbudowę o dodatkowe czujniki, system wizyjny i inne narzędzia robocze.

Projekt ma charakter edukacyjny i eksperymentalny. Nie jest urządzeniem przemysłowym ani certyfikowanym robotem przeznaczonym do pracy w środowisku produkcyjnym.

---

# 2. CHARAKTERYSTYKA ROBOTA

Robot jest manipulatorem typu SCARA, którego główna część wykonawcza składa się z:

- osi obrotowej ARM1,
- osi obrotowej ARM2,
- liniowej osi pionowej Z,
- serwomechanizmu obrotu chwytaka,
- serwomechanizmu otwierania i zamykania chwytaka.

Za ruch trzech głównych osi odpowiadają silniki krokowe NEMA 17. Ruch obrotowy chwytaka i jego otwieranie realizują serwomechanizmy.

Konstrukcja mechaniczna bazuje na rozwiązaniach projektu PyBot i wykorzystuje elementy wykonane metodą druku 3D oraz standardowe elementy mechaniczne stosowane m.in. w drukarkach 3D.

Sterowanie realizuje mikrokontroler ESP32 współpracujący z dedykowaną płytką PCB. ESP32 komunikuje się z bezprzewodowym kontrolerem poprzez Bluetooth i samodzielnie realizuje logikę sterowania, kinematykę, generację trajektorii PTP, obsługę TEACH/AUTO oraz zabezpieczenia programowe.

Komputer nie jest wymagany do normalnej pracy robota. Jest wykorzystywany do programowania, aktualizacji firmware oraz diagnostyki przez port szeregowy.

---

# 3. PARAMETRY TECHNICZNE

## 3.1. Parametry konstrukcji i sterowania

| Parametr | Wartość |
|---|---:|
| Typ robota | SCARA |
| Liczba głównych osi | 3 |
| Osie robocze | ARM1, ARM2, Z |
| Dodatkowe osie chwytaka | 2 |
| Silniki główne | 3 × NEMA 17 |
| Kąt pełnego kroku silnika | 1,8° |
| Liczba pełnych kroków na obrót | 200 |
| Mikrokrok | 1/16 |
| Serwomechanizmy | 2 × klasy SG90/MG90/MG92R |
| Sterownik główny | ESP32 D1 Mini |
| Sterowniki silników | A4988 |
| Środowisko programistyczne | PlatformIO / Visual Studio Code |
| Framework | Arduino |
| Język programu | C++ |
| Komunikacja z padem | Bluetooth / Bluepad32 |

## 3.2. Parametry wykorzystywane przez aktualny firmware

| Parametr | Wartość |
|---|---:|
| Długość członu ARM1 | 91,61 mm |
| Długość członu ARM2 | 97,528 mm |
| Przełożenie ARM1 | 4,5 |
| Przełożenie ARM2 | 7,280 |
| Skok śruby osi Z | 2,0 mm/obr. |
| Przełożenie Z | 1,0 |
| Zakres programowy ARM1 | −105° ... +105° |
| Zakres programowy ARM2 | −125° ... +125° |
| Zakres programowy Z | 0 ... 300 mm |
| Zakres obrotu chwytaka | 0 ... 180° |
| Zakres serwa chwytaka | 15 ... 95° |
| Prędkość ręczna ARM1 | 25°/s |
| Prędkość ręczna ARM2 | 25°/s |
| Prędkość ręczna Z | 3,0 mm/s |
| Prędkość TCP w trybie TOOL | 20 mm/s |
| Bazowa prędkość AUTO ARM1 | 20°/s |
| Bazowa prędkość AUTO ARM2 | 20°/s |
| Bazowa prędkość AUTO Z | 3,0 mm/s |
| Override prędkości AUTO | 50...250% dla ARM1, ARM2 i Z |
| Przyspieszenie ARM1 | 12000 STEP/s² |
| Przyspieszenie ARM2 | 5000 STEP/s² |
| Przyspieszenie Z | 6000 STEP/s² |
| Liczba punktów programu TEACH | maks. 20 |

## 3.3. Parametry wymagające pomiaru rzeczywistego egzemplarza

Nie należy bez dodatkowych pomiarów przyjmować wartości deklarowanych dla bazowego projektu PyBot jako parametrów gotowego robota.

W szczególności wymagają pomiaru:

- rzeczywista masa robota,
- rzeczywista przestrzeń robocza,
- rzeczywista powtarzalność pozycjonowania,
- maksymalny bezpieczny udźwig,
- maksymalne praktyczne prędkości osi,
- dokładność pozycjonowania TCP.

Do czasu wykonania prób odbiorczych wartości te powinny być traktowane jako „do wyznaczenia”.

---

# 4. BUDOWA MECHANICZNA

## 4.1. Podstawa

Podstawa stanowi element nośny całej konstrukcji. Mocowane są do niej m.in.:

- prowadnice osi Z,
- napędy,
- elementy przekładni pasowych,
- elektronika sterująca,
- elementy zabezpieczające przewody.

Podstawa powinna zapewniać sztywność konstrukcji oraz możliwość stabilnego ustawienia robota na stanowisku.

## 4.2. Pierwszy człon ramienia – ARM1

Pierwszy człon łączy podstawę z centralnym przegubem robota. Jego ruch zmienia kierunek całego manipulatora w płaszczyźnie XY.

Napęd realizowany jest silnikiem krokowym poprzez przekładnię pasową.

## 4.3. Drugi człon ramienia – ARM2

Drugi człon łączy pierwszy człon z końcówką roboczą. Wspólnie z osią ARM1 określa położenie TCP w płaszczyźnie XY.

Ruch realizowany jest niezależnym napędem krokowym poprzez układ przekładni pasowych.

## 4.4. Oś pionowa Z

Oś Z realizuje ruch pionowy końcówki roboczej. Napęd odbywa się za pomocą silnika krokowego i śruby o skoku 2 mm/obrót.

Ze względu na dużą liczbę kroków przypadających na 1 mm ruchu oraz ryzyko gubienia kroków, oś Z pracuje z wyraźnie niższą prędkością niż osie obrotowe.

## 4.5. Chwytak

Chwytak jest wyposażony w dwa serwomechanizmy:

- serwo obrotu narzędzia,
- serwo otwierania i zamykania chwytaka.

Położenia serw mogą być zapisywane razem z punktami programu TEACH.

---

# 5. UKŁAD NAPĘDOWY

## 5.1. Silniki krokowe

Każda z trzech głównych osi korzysta z silnika NEMA 17 o kącie pełnego kroku 1,8°.

Pełny obrót odpowiada:

`360° / 1,8° = 200 kroków`

Przy zastosowaniu mikrokroku 1/16:

`200 × 16 = 3200 mikrokroków na obrót silnika`

Dodatkowo liczba kroków roboczych jest zależna od przełożenia danej osi.

Dla osi Z, przy skoku śruby 2 mm/obrót i mikrokroku 1/16:

`3200 / 2 = 1600 kroków/mm`

Wartość ta jest wykorzystywana przez firmware do przeliczania położenia osi Z.

## 5.2. Sterowniki A4988

Sterowniki A4988 odpowiadają za zasilanie i sterowanie silników krokowych. Każdy sterownik otrzymuje z ESP32 sygnały:

- STEP,
- DIR.

Należy prawidłowo ustawić prąd sterowników zgodnie z zastosowanymi silnikami i chłodzeniem.

Nie wolno podłączać ani odłączać silnika krokowego przy załączonym zasilaniu sterownika.

---

# 6. UKŁAD ELEKTRYCZNY I ELEKTRONICZNY

## 6.1. Główne elementy

Układ elektroniczny zawiera:

- ESP32 D1 Mini,
- 3 sterowniki A4988,
- przetwornicę LM2596,
- złącza silników,
- złącza serwomechanizmów,
- przyciski TEACH i MODE,
- obwód E-STOP,
- diody sygnalizacyjne,
- wyświetlacz OLED SSD1306 128×64 I2C,
- złącza krańcówek,
- elementy bierne i zabezpieczające.

Dokumentacja płytki PCB znajduje się w katalogu `PCB` repozytorium.

## 6.2. Przypisanie wejść i wyjść ESP32

| Funkcja | GPIO |
|---|---:|
| STEP ARM1 | 13 |
| DIR ARM1 | 26 |
| STEP ARM2 | 23 |
| DIR ARM2 | 14 |
| STEP Z | 18 |
| DIR Z | 19 |
| Servo ROTATE | 27 |
| Servo GRIP | 32 |
| TEACH | 25 |
| MODE | 33 |
| E-STOP | 39 |
| Krańcówka Z | 34 |
| Krańcówka ARM1 | 35 |
| Krańcówka ARM2 | 36 |
| LED Bluetooth | 4 |
| LED ERROR | 16 |
| LED STATUS | 17 |
| I2C SDA (OLED) | 21 |
| I2C SCL (OLED) | 22 |

## 6.3. Krańcówki

Płytka posiada wejścia dla krańcówek osi ARM1, ARM2 i Z.

W aktualnej wersji firmware krańcówki nie realizują automatycznego homingu. Wejścia zostały zachowane sprzętowo na potrzeby przyszłej rozbudowy.

## 6.4. E-STOP

E-STOP jest elementem bezpieczeństwa układu.

Po jego aktywacji firmware:

- natychmiast zatrzymuje generowanie kroków,
- blokuje sterowanie ręczne,
- ustawia program AUTO w stan FAULT,
- sygnalizuje błąd,
- wymaga restartu sterownika przed ponowną pracą.

Układ powinien być wykonany tak, aby E-STOP nie opierał się wyłącznie na programie, lecz dodatkowo ograniczał możliwość dalszego zasilania lub aktywacji napędów zgodnie z założeniami płytki sterującej.

## 6.5. Wyświetlacz OLED

Sterownik obsługuje wyświetlacz OLED SSD1306 128×64 podłączony przez I2C. Magistrala wykorzystuje GPIO21 jako SDA i GPIO22 jako SCL.

Podczas inicjalizacji firmware automatycznie sprawdza adresy `0x3C` i `0x3D`. Brak wyświetlacza nie blokuje pracy robota — informacja o problemie jest wysyłana przez Serial.

W czasie pracy OLED pokazuje m.in.:

- tryb MANUAL/AUTO i stan programu,
- stan połączenia Bluetooth,
- liczbę zapisanych punktów TEACH,
- override prędkości w AUTO,
- aktualne położenia ARM1, ARM2 i Z,
- pozycję TCP X/Y lub numer aktualnego punktu programu,
- stany błędów, w tym `E-STOP` i `NUM ERR`.

---

# 7. ARCHITEKTURA STEROWANIA

Rzeczywisty przepływ sterowania można przedstawić następująco:

`Pad Bluetooth → Bluepad32 → ESP32 → logika sterowania → kinematyka / AUTO → FastAccelStepper → A4988 → silniki NEMA17`

Równolegle:

`ESP32 → ESP32Servo → serwo obrotu + serwo chwytaka`

ESP32 pełni jednocześnie funkcję:

- odbiornika poleceń,
- sterownika osi,
- sterownika serw,
- układu kinematyki,
- generatora sekwencji AUTO,
- pamięci programu TEACH,
- modułu bezpieczeństwa programowego,
- układu diagnostycznego.

---

# 8. OPROGRAMOWANIE

## 8.1. Środowisko

Program sterujący został przygotowany w języku C++ z wykorzystaniem:

- Arduino Framework,
- PlatformIO,
- Visual Studio Code.

## 8.2. Główne biblioteki

### Bluepad32

Biblioteka odpowiada za obsługę kontrolerów Bluetooth.

### FastAccelStepper

Biblioteka generuje sygnały STEP dla trzech osi i umożliwia definiowanie prędkości, przyspieszeń oraz pozycji.

### ESP32Servo

Biblioteka steruje serwomechanizmami chwytaka.

### Preferences

Biblioteka obsługuje pamięć NVS ESP32. W pamięci zapisywany jest program TEACH, dzięki czemu punkty pozostają dostępne po wyłączeniu zasilania.

### Adafruit GFX / Adafruit SSD1306

Biblioteki odpowiadają za obsługę wyświetlacza OLED SSD1306 128×64, prezentację stanu robota oraz podstawową diagnostykę.

---

# 9. TRYBY PRACY

Sterownik wykorzystuje dwa niezależne poziomy trybów.

## 9.1. Tryb pracy głównej

- `MANUAL`
- `AUTO`

## 9.2. Tryb sterowania ręcznego

W trybie MANUAL dostępne są:

- `JOINT`
- `TOOL`

Dzięki temu wybór JOINT/TOOL nie jest tym samym co wybór MANUAL/AUTO.

---

# 10. STEROWANIE RĘCZNE – JOINT

W trybie JOINT operator steruje bezpośrednio poszczególnymi osiami robota.

Aktualne mapowanie pada:

| Element kontrolera | Funkcja |
|---|---|
| Lewy analog X | ARM1 |
| Lewy analog Y | ARM2 |
| Prawy analog Y | Z |
| Prawy analog X | obrót chwytaka |
| A / B | otwieranie / zamykanie chwytaka |
| START | JOINT / TOOL |

Wartości z analogów są normalizowane i posiadają martwą strefę, aby niewielkie odchylenia drążka nie powodowały ruchu robota.

Po połączeniu kontrolera robot nie zostaje od razu uzbrojony. Wymagane jest około 300 ms neutralnego położenia wszystkich elementów sterujących. Rozwiązanie to zapobiega niekontrolowanemu ruchowi tuż po sparowaniu pada.

---

# 11. STEROWANIE RĘCZNE – TOOL

W trybie TOOL operator nie steruje bezpośrednio kątami ARM1 i ARM2.

Lewy analog zadaje ruch końcówki roboczej w płaszczyźnie XY, a program oblicza wymagane kąty ramion za pomocą kinematyki odwrotnej.

Sterowanie osi Z i chwytakiem pozostaje analogiczne jak w trybie JOINT.

Jeżeli zadany punkt jest poza obszarem osiągalnym lub wymaga przekroczenia limitu osi, polecenie nie jest wykonywane i może zostać zasygnalizowany błąd.

---

# 12. KINEMATYKA

## 12.1. Kinematyka prosta

Dla dwóch ramion SCARA położenie TCP w płaszczyźnie XY wyznaczane jest z zależności:

`X = L1·cos(θ1) + L2·cos(θ1 + θ2)`

`Y = L1·sin(θ1) + L2·sin(θ1 + θ2)`

oraz:

`Z = położenie osi Z`

gdzie:

- `L1` – długość pierwszego członu,
- `L2` – długość drugiego członu,
- `θ1` – kąt ARM1,
- `θ2` – kąt ARM2.

W aktualnej konfiguracji:

- `L1 = 91,61 mm`,
- `L2 = 97,528 mm`.

## 12.2. Kinematyka odwrotna

W trybie TOOL zadawana jest pozycja `(X,Y)`, a program oblicza kąty `θ1` i `θ2`.

Najpierw obliczana jest zależność:

`cos(θ2) = (X² + Y² − L1² − L2²) / (2·L1·L2)`

Jeżeli wynik znajduje się poza zakresem `−1...1`, punkt leży poza geometrycznym obszarem pracy.

Następnie wyznaczane są kąty przegubów. Dla typowego robota dwuprzegubowego istnieją dwie konfiguracje geometryczne, odpowiadające położeniu „łokcia” po dwóch stronach.

Aktualny firmware wykorzystuje jedną z tych konfiguracji (`ELBOW_UP`).

Po obliczeniu kątów sprawdzane są również programowe ograniczenia zakresów ARM1 i ARM2.

---

# 13. POZYCJA HOME I HOMING

Aktualny robot nie wykonuje automatycznego homingu.

Pozycję HOME należy ustawić ręcznie przed włączeniem zasilania lub resetem ESP32.

Firmware przyjmuje następujące wartości startowe:

| Oś | Pozycja HOME |
|---|---:|
| ARM1 | 0° |
| ARM2 | 0° |
| Z | 0 mm |
| Obrót chwytaka | 90° |
| Chwytak | 30° |

Po starcie licznik kroków każdej osi zostaje ustawiony na pozycję odpowiadającą HOME.

Jeżeli mechanika nie została rzeczywiście ustawiona w tej pozycji, wszystkie późniejsze obliczenia położenia będą obarczone błędem.

Automatyczny homing z wykorzystaniem krańcówek jest przewidywany jako możliwe przyszłe rozszerzenie.

---

# 14. FUNKCJA TEACH

Przycisk TEACH na płytce umożliwia zapis aktualnej pozycji robota.

TEACH działa tylko:

- w trybie MANUAL,
- przy zatrzymanych osiach,
- gdy E-STOP nie jest aktywny.

## 14.1. Krótkie naciśnięcie

Zapisuje kolejny punkt programu.

Każdy punkt zawiera:

- pozycję ARM1,
- pozycję ARM2,
- pozycję Z,
- pozycję serwa obrotu chwytaka,
- pozycję serwa chwytaka,
- czas postoju `dwellMs`.

Maksymalna liczba punktów wynosi 20.

## 14.2. Długie naciśnięcie

Przytrzymanie przycisku przez około 1,8 s usuwa cały zapisany program.

## 14.3. Pamięć

Program jest przechowywany w pamięci NVS ESP32. Dzięki temu nie jest tracony po wyłączeniu zasilania.

---

# 15. TRYB AUTO

Przycisk MODE przełącza pracę pomiędzy MANUAL i AUTO.

Zmiana trybu jest możliwa tylko przy zatrzymanych osiach.

W AUTO pad nie służy do bezpośredniego sterowania osiami.

Przycisk START realizuje funkcję:

- IDLE → START,
- RUN → PAUSE,
- PAUSED → RESUME.

AUTO odtwarza punkty zapisane wcześniej funkcją TEACH.

---

# 16. PRZEBIEG CYKLU AUTO

Dla każdego punktu programu wykonywane są następujące działania:

1. ustawienie pozycji docelowych serwomechanizmów,
2. wyznaczenie odległości do przejechania przez ARM1, ARM2 i Z,
3. dobranie prędkości osi,
4. rozpoczęcie ruchu PTP,
5. oczekiwanie na zakończenie ruchu wszystkich osi,
6. odczekanie zadanego czasu postoju,
7. przejście do kolejnego punktu.

W aktualnej konfiguracji program pracuje domyślnie w pętli.

## 16.1. Typ ruchu

Obecna implementacja AUTO realizuje ruch PTP (Point To Point).

Oznacza to, że synchronizowane są czasy przejazdu osi, ale tor TCP pomiędzy punktami nie musi być linią prostą.

Interpolacja liniowa LIN jest przewidywana jako możliwe przyszłe rozszerzenie.

---

# 17. REGULACJA PRĘDKOŚCI W AUTO

W gałęzi `dev` dostępna jest regulacja prędkości całego ruchu w trybie AUTO.

| D-pad | Funkcja |
|---|---|
| góra | +10% |
| dół | −10% |
| lewo | powrót do 100% |

Zakres regulacji:

`50% ... 250%`

Override skaluje bazowe prędkości wszystkich trzech osi: ARM1, ARM2 i Z. Przy 100% wartości bazowe wynoszą odpowiednio 20°/s, 20°/s i 3,0 mm/s.

Zmiana prędkości zaczyna obowiązywać od kolejnego ruchu PTP. Bieżący ruch nie jest przeliczany w locie.

---


# 18. ZABEZPIECZENIA PROGRAMOWE

Firmware zawiera kilka warstw zabezpieczeń i kontroli poprawności danych.

## 18.1. Limity pozycji

Polecenia ruchu są ograniczane do zakresów:

- ARM1: −105° ... +105°,
- ARM2: −125° ... +125°,
- Z: 0 ... 300 mm.

## 18.2. Częstotliwość STEP i prędkość

Aktualny firmware nie stosuje dodatkowych twardych górnych limitów częstotliwości STEP. Częstotliwość STEP wynika z zadanej prędkości osi, przełożenia, mikrokroku oraz override AUTO.

W MANUAL prędkość jest ograniczana do skonfigurowanych prędkości JOG, natomiast w AUTO do bazowych prędkości AUTO pomnożonych przez override 50...250%. Przyspieszenia są realizowane przez FastAccelStepper.

## 18.3. Walidacja wartości numerycznych

Przed przeliczeniami i wydaniem części poleceń ruchu sprawdzane są wartości typu `NaN` i `Inf`. W przypadku wykrycia błędu firmware nie przekazuje nieprawidłowej pozycji do napędów, może zsynchronizować pozycję z licznikami kroków i zgłasza błąd przez Serial/OLED.

## 18.4. Walidacja programu TEACH/NVS

Podczas odczytu programu z NVS sprawdzany jest rozmiar danych oraz poprawność zapisanych punktów. Nieprawidłowy program, np. zawierający `NaN`, `Inf` lub pozycję poza zakresem osi, jest odrzucany i czyszczony.

## 18.5. Timeout pada

W MANUAL brak aktualnych danych z kontrolera przez określony czas powoduje zatrzymanie ruchu i rozbrojenie sterowania.

## 18.6. Neutralne uzbrojenie pada

Po połączeniu kontrolera wymagane jest neutralne położenie elementów sterujących przez około 300 ms przed uzbrojeniem ruchu.

## 18.7. Utrata kontrolera w AUTO

Rozłączenie kontrolera podczas aktywnego cyklu AUTO powoduje przejście programu w PAUSE.

## 18.8. E-STOP

E-STOP zatrzaskuje stan awaryjny do czasu restartu sterownika. Programowo zatrzymywane są generatory kroków, AUTO przechodzi w FAULT, a sterowanie ręczne pozostaje zablokowane.

---


# 19. PROCEDURA MONTAŻU ELEKTRONIKI

1. Sprawdzić PCB pod kątem zwarć, uszkodzeń ścieżek i jakości lutowania.
2. Zweryfikować polaryzację kondensatorów elektrolitycznych i diod.
3. Sprawdzić orientację tranzystorów, układów scalonych i sterowników A4988.
4. Zamontować ESP32 na przewidzianych złączach goldpin.
5. Przed podłączeniem ESP32 i serw ustawić napięcie LM2596 na 5,0 V.
6. Podłączyć przewody silników, serw i elementów panelu zgodnie ze schematem.
7. Uporządkować przewody tak, aby nie kolidowały z ruchomymi częściami.
8. Zapewnić luz przewodów w miejscach ruchu.
9. Zapewnić wentylację A4988 oraz elementów wydzielających ciepło.
10. Sprawdzić działanie E-STOP przed testami ruchu.

---

# 20. PROCEDURA URUCHOMIENIA

Przed pierwszym uruchomieniem:

1. sprawdzić wszystkie połączenia mechaniczne,
2. sprawdzić naciąg pasków,
3. sprawdzić prowadzenie przewodów,
4. sprawdzić poprawność zasilania,
5. ustawić LM2596 na 5,0 V,
6. sprawdzić ustawienie prądów A4988,
7. upewnić się, że obszar pracy robota jest wolny,
8. ręcznie ustawić robot w pozycji HOME,
9. włączyć zasilanie,
10. obserwować diody sygnalizacyjne i komunikaty Serial,
11. połączyć kontroler Bluetooth,
12. pozostawić analogi i przyciski w neutralnym położeniu do czasu uzbrojenia,
13. wykonać krótkie testy pojedynczych osi przy małych wychyleniach analogów,
14. sprawdzić kierunki ruchu,
15. sprawdzić działanie E-STOP,
16. dopiero po poprawnym wykonaniu powyższych testów przejść do pełnego zakresu pracy.

---

# 21. TESTY ODBIORCZE

Zamiast ograniczać testy do stwierdzenia „działa poprawnie”, zaleca się zapisywanie wyników pomiarowych.

## 21.1. Kontrola PCB

**Kryterium:** brak zwarć, zimnych lutów, błędnej orientacji i uszkodzeń.

## 21.2. Kontrola napięcia 5 V

**Kryterium:** napięcie LM2596 ustawione na 5,0 V przed podłączeniem odbiorników wymagających tego napięcia.

## 21.3. Test ARM1

Zadać ruch np. o 30°, 60° i 90° w obu kierunkach.

**Sprawdzić:**

- kierunek,
- płynność,
- brak gubienia kroków,
- zgodność kąta rzeczywistego z zadanym.

## 21.4. Test ARM2

Analogicznie jak dla ARM1.

## 21.5. Test osi Z

Wykonać przejazdy kontrolne np. 5 mm, 10 mm i 20 mm.

Pomiar wykonać suwmiarką lub czujnikiem zegarowym.

## 21.6. Test chwytaka

Sprawdzić:

- pełne otwarcie,
- pełne zamknięcie,
- brak mechanicznego blokowania,
- brak nadmiernego poboru prądu przy dojeździe do ograniczeń.

## 21.7. Test TEACH

Zapisać kilka punktów, wyłączyć zasilanie, uruchomić ponownie i zweryfikować obecność programu.

## 21.8. Test AUTO

Zapisać minimum 5 punktów i uruchomić wielokrotne odtwarzanie.

Sprawdzić:

- osiąganie wszystkich punktów,
- poprawną obsługę serw,
- czasy postoju,
- działanie START/PAUSE/RESUME,
- poprawne przejście do kolejnego cyklu.

## 21.9. Test utraty kontrolera

### MANUAL

Rozłączenie pada powinno zatrzymać sterowanie.

### AUTO

Utrata kontrolera podczas aktywnego cyklu powinna przełączyć program w PAUSE.

## 21.10. Test E-STOP

Po aktywacji E-STOP:

- wszystkie osie powinny zostać zatrzymane,
- stan AUTO powinien przejść w FAULT,
- dalsza praca powinna być niemożliwa do czasu restartu.

## 21.11. Test powtarzalności

Wybrać minimum trzy punkty w przestrzeni roboczej.

Dla każdego wykonać co najmniej 20 dojazdów i zmierzyć rozrzut pozycji TCP.

Na podstawie tych pomiarów można dopiero wpisać rzeczywistą powtarzalność robota.

## 21.12. Test długotrwały

Zaleca się wykonanie co najmniej 30-minutowej pracy cyklicznej.

Należy monitorować:

- temperaturę A4988,
- temperaturę silników,
- temperaturę LM2596,
- stabilność Bluetooth,
- ewentualne gubienie kroków,
- samoczynne resety ESP32.

---

# 22. BEZPIECZEŃSTWO

Podczas pracy nie wolno:

- wkładać rąk w obszar pracy robota,
- zatrzymywać ramienia ręką,
- dotykać poruszających się pasków,
- pracować z uszkodzonymi przewodami,
- podłączać i odłączać silników krokowych przy załączonym zasilaniu,
- pozostawiać robota bez nadzoru podczas testów,
- uruchamiać pełnej prędkości przed zakończeniem testów pojedynczych osi.

Przed pracami serwisowymi należy odłączyć zasilanie.

Ze względu na edukacyjny charakter urządzenia wszystkie testy powinny być prowadzone pod nadzorem osoby znającej budowę stanowiska.

---

# 23. KONSERWACJA

Okresowo należy kontrolować:

- naciąg pasków,
- stan kół pasowych,
- stan łożysk,
- stan prowadnic,
- dokręcenie śrub,
- stan elementów drukowanych 3D,
- prowadzenie przewodów,
- temperaturę silników,
- temperaturę A4988,
- stabilność połączeń złączowych.

Szczególną uwagę należy zwracać na układ napędowy drugiego członu, ponieważ luz paska bezpośrednio wpływa na dokładność i powtarzalność robota.

---

# 24. MOŻLIWOŚCI ROZBUDOWY

Projekt może zostać rozwinięty o:

- automatyczny homing z wykorzystaniem krańcówek,
- interpolację liniową LIN w przestrzeni TCP,
- system wizyjny,
- kamerę do rozpoznawania obiektów,
- czujnik odległości,
- wykrywanie obecności detalu,
- elektromagnes,
- przyssawkę podciśnieniową,
- marker,
- dozownik,
- narzędzie pomiarowe,
- interfejs PC do edycji programów,
- zapis większej liczby programów,
- wybór programu z poziomu panelu,
- raportowanie pozycji i błędów przez Wi-Fi.

---

# 25. PRZYKŁADOWA SEKWENCJA PICK AND PLACE

Przykładowa sekwencja może wyglądać następująco:

`START → pozycja bazowa → przejazd nad detal → Z w dół → zamknięcie chwytaka → Z w górę → przejazd nad miejsce odkładania → Z w dół → otwarcie chwytaka → Z w górę → powrót → STOP`

Sekwencję można zapisać za pomocą funkcji TEACH, a następnie odtwarzać w trybie AUTO.

---

# 26. ZALEŻNOŚĆ PARAMETRÓW MECHANICZNYCH I PROGRAMOWYCH

Zmiana konstrukcji mechanicznej może wymagać modyfikacji firmware.

Przykładowo:

`zmiana długości ramienia → zmiana parametrów kinematyki → zmiana obliczanych kątów → zmiana położenia TCP`

Podobnie:

`zmiana przełożenia → zmiana liczby kroków/stopień → błędna pozycja osi przy starych parametrach`

Dlatego po każdej modyfikacji:

- długości ramion,
- kół pasowych,
- przełożeń,
- śruby osi Z,
- mikrokroku,

należy sprawdzić i w razie potrzeby zaktualizować parametry programu.

---

# 27. WERSJONOWANIE DOKUMENTACJI

Dokumentacja techniczna powinna zawsze wskazywać wersję firmware lub commit repozytorium, do którego się odnosi.

Jest to szczególnie ważne, ponieważ w trakcie rozwoju projektu zmieniały się m.in.:

- prędkości osi,
- przyspieszenia,
- zachowanie przycisków MODE i TEACH,
- obsługa utraty połączenia z padem,
- parametry AUTO,
- zabezpieczenia przed niekontrolowanym ruchem.

Przed wydrukowaniem dokumentacji zaleca się wpisanie:

- numeru rewizji PCB,
- nazwy gałęzi,
- hash commita,
- daty wersji,
- autora dokumentacji.

---

# 28. POCHODZENIE ELEMENTÓW PROJEKTU

Konstrukcja mechaniczna korzysta z rozwiązań i modeli projektu bazowego PyBot, natomiast układ sterowania, płytka PCB, konfiguracja ESP32 i aktualny firmware są rozwijane w ramach niniejszego projektu.

Przed publiczną dystrybucją kompletu materiałów należy zweryfikować warunki licencyjne modeli źródłowych oraz dodać do repozytorium odpowiednie informacje o licencjach i autorach.

---

# 29. PODSUMOWANIE

Projekt „Robotyka bez granic” stanowi kompletne edukacyjne stanowisko mechatroniczne łączące mechanikę, druk 3D, elektronikę, napędy, programowanie mikrokontrolerów, kinematykę robotów oraz zagadnienia automatyzacji.

Aktualna wersja robota umożliwia:

- sterowanie bezprzewodowe,
- ręczne prowadzenie osi w trybie JOINT,
- sterowanie TCP w trybie TOOL,
- zapisywanie punktów TEACH,
- automatyczne odtwarzanie programu PTP,
- regulację prędkości ARM1, ARM2 i Z w AUTO,
- bieżący podgląd stanu na wyświetlaczu OLED,
- trwałe przechowywanie programu w pamięci ESP32,
- zatrzymanie awaryjne oraz dodatkowe zabezpieczenia programowe.

Najważniejszymi dalszymi kierunkami rozwoju są automatyczny homing, interpolacja LIN, pomiar rzeczywistej powtarzalności i udźwigu oraz dalsza rozbudowa interfejsu operatora.
