# AUTO / TEACH – aktualny firmware `main`

## Pozycja HOME

Robot nie wykonuje automatycznego homingu wszystkich osi.

Pozycja startowa jest ustalana w dwóch etapach:

- ARM1: operator ręcznie ustawia mechanikę na `0°` przed włączeniem zasilania lub resetem,
- ARM2: operator ręcznie ustawia mechanikę na `0°` przed włączeniem zasilania lub resetem,
- Z: po restarcie operator ustawia fizyczne zero osią sterowaną z pada i zatwierdza je kombinacją `A+B`.

### Startowe ustawianie Z HOME

Po uruchomieniu sterownika:

1. OLED wyświetla logo przez 5 s.
2. Następnie pojawia się ekran `SET Z HOME`.
3. Pad musi być połączony i uzbrojony przez pozostawienie wszystkich wejść w neutralnym położeniu przez 300 ms.
4. Prawy analog Y steruje wyłącznie osią Z z prędkością maksymalną `1.5 mm/s`.
5. Po puszczeniu analogu Z jest natychmiast zatrzymywana.
6. Po ustawieniu mechanicznego zera operator, przy zatrzymanej osi, naciska jednocześnie `A+B`.
7. Bieżąca pozycja licznika kroków zostaje wtedy ustawiona jako `Z = 0.00 mm`.
8. Pad jest ponownie rozbrajany; po puszczeniu `A+B` trzeba pozostawić go neutralnie przez 300 ms.

Do chwili zatwierdzenia Z HOME normalne sterowanie MANUAL/AUTO oraz przyciski MODE i TEACH są zablokowane.

## Tryby pracy

Sterownik ma dwa niezależne poziomy trybów:

- `OperatingMode::MANUAL` / `OperatingMode::AUTO`,
- w MANUAL: `ControlMode::JOINT` / `ControlMode::TOOL`.

JOINT/TOOL określa sposób ręcznego prowadzenia robota, a AUTO jest osobnym trybem pracy.

## Przyciski na PCB

### MODE

Przełącza `MANUAL <-> AUTO`.

Zmiana jest odrzucana, gdy którakolwiek oś jest w ruchu. Po przejściu z AUTO do MANUAL pad musi ponownie przejść neutralne uzbrojenie.

### TEACH

Działa tylko w MANUAL i przy zatrzymanych osiach.

- krótkie naciśnięcie: zapis aktualnego punktu,
- przytrzymanie `>= 1.8 s`: wyczyszczenie całego programu.

Punkt zapisuje:

- rzeczywistą pozycję ARM1,
- rzeczywistą pozycję ARM2,
- rzeczywistą pozycję Z,
- pozycję serwa obrotu narzędzia,
- pozycję serwa 2 / chwytaka,
- czas postoju `dwellMs`.

Program może zawierać maksymalnie 20 punktów. Punkty są przechowywane w NVS ESP32 i pozostają po wyłączeniu zasilania.

## Pad – MANUAL

`START` przełącza JOINT / TOOL.

### JOINT

- lewy analog X – ARM1,
- lewy analog Y – ARM2,
- prawy analog Y – Z,
- prawy analog X – obrót narzędzia,
- A – zwiększa kąt serwa 2,
- B – zmniejsza kąt serwa 2.

### TOOL

- lewy analog X/Y – ruch TCP w X/Y przez kinematykę odwrotną,
- prawy analog Y – Z,
- prawy analog X – obrót narzędzia,
- A/B – zmiana kąta serwa 2.

Po połączeniu pada wymagane jest 300 ms neutralnych wejść przed uzbrojeniem ruchu.

### Zakresy serw

- serwo obrotu narzędzia: `0 ... 180°`, pozycja startowa `90°`,
- serwo 2 / chwytak: `90 ... 180°`, pozycja startowa `90°`.

## Pad – AUTO

`START` działa jako:

- IDLE -> START,
- RUN -> PAUSE,
- PAUSED -> RESUME.

Regulacja prędkości całego ruchu w AUTO:

- D-pad góra: `+10%`,
- D-pad dół: `-10%`,
- D-pad lewo: powrót do `100%`,
- zakres regulacji: `50% ... 250%`,
- regulacja skaluje ARM1, ARM2 oraz Z,
- nowa wartość obowiązuje od następnego punktu PTP; bieżący ruch nie jest przeliczany w locie.

Bazowo oba ramiona w AUTO mają `20 deg/s`, a oś Z `3.0 mm/s`. Przykładowo przy `200%` ramiona mają bazową prędkość `40 deg/s`, a Z `6.0 mm/s`.

Sterownik nie nakłada dodatkowego programowego limitu częstotliwości STEP; rzeczywista granica wynika z możliwości napędu, sterownika i mechaniki.

Utrata pada podczas AUTO przełącza program w PAUSE.

## Cykl AUTO

AUTO odtwarza zapisane punkty jako ruch PTP. Dla każdego punktu:

1. ustawiane są cele serw,
2. obliczane są odległości wszystkich trzech osi,
3. dobierane są częstotliwości STEP tak, aby osie w przybliżeniu kończyły ruch w tym samym czasie,
4. sterownik czeka nieblokująco na zakończenie wszystkich osi,
5. wykonywany jest `dwellMs`,
6. uruchamiany jest kolejny punkt.

Domyślnie program pracuje w pętli (`AUTO_LOOP_PROGRAM = true`).

## Sygnalizacja

- LED Bluetooth: świeci ciągle przy połączonym padzie, miga przy braku połączenia,
- LED STATUS: świeci w trybie AUTO,
- LED ERROR: sygnalizuje m.in. timeout pada, błąd IK, E-STOP lub FAULT AUTO,
- OLED: pokazuje tryb, stan pada, położenia osi, liczbę punktów i stan AUTO.

## Bezpieczeństwo

- E-STOP natychmiast zatrzymuje generowanie kroków i ustawia AUTO w `FAULT`.
- Po E-STOP wymagany jest restart sterownika.
- AUTO nie jest zależne od `gamepadArmed`; blokada `gamepadArmed` dotyczy ruchu ręcznego.
- Do AUTO wykorzystywana jest pozycja liczona z `FastAccelStepper::getCurrentPosition()`.
- Sterownik nie ma enkoderów osi, dlatego utrata kroków powoduje utratę zgodności pozycji programowej z mechaniczną.
- Po każdym restarcie należy ponownie wykonać procedurę Z HOME.

## Kolejny etap

Obecna wersja AUTO realizuje PTP. Możliwym kolejnym rozszerzeniem jest interpolacja `LIN`, w której TCP jest dzielony na krótkie odcinki XYZ, a dla kolejnych próbek wykonywana jest kinematyka odwrotna.
