# Dokumentacja projektu SCARA

Ten katalog zawiera dokumentację eksploatacyjną i techniczną robota SCARA.

## Najważniejsze pliki

- [`INSTRUKCJA_OBSLUGI_PL.md`](INSTRUKCJA_OBSLUGI_PL.md) – instrukcja uruchomienia i obsługi robota.
- [`AUTO_TEACH.md`](AUTO_TEACH.md) – skrócony opis trybów MANUAL, TOOL, TEACH i AUTO.
- [`DOKUMENTACJA_TECHNICZNA_PL.md`](DOKUMENTACJA_TECHNICZNA_PL.md) – pełna dokumentacja techniczna w języku polskim.
- [`DOKUMENTACJA_TECHNICZNA_UA.md`](DOKUMENTACJA_TECHNICZNA_UA.md) – pełna dokumentacja techniczna w języku ukraińskim.
- `DOKUMENTACJA_TECHNICZNA_PL.docx` i `DOKUMENTACJA_TECHNICZNA_UA.docx` – wersje Word przygotowane do druku i dalszej edycji.

## Aktualne zachowanie firmware

Aktualna gałąź `main` zawiera m.in.:

- ręczne ustawianie ARM1 i ARM2 na `0°` przed startem,
- startową procedurę ręcznego HOME osi Z z pada,
- potwierdzenie `Z = 0` przez jednoczesne naciśnięcie `A+B`,
- blokadę normalnej pracy do zatwierdzenia HOME Z,
- logo na OLED przez 5 s po uruchomieniu,
- tryby MANUAL JOINT, MANUAL TOOL i AUTO,
- zapis maksymalnie 20 punktów TEACH do NVS,
- automatyczne odtwarzanie PTP,
- regulację prędkości AUTO 50–250%,
- serwo 2 startujące z `90°` i ograniczone programowo do `90–180°`,
- komentarze w kodzie w językach polskim, angielskim i ukraińskim.

## Ważna uwaga o pozycji osi

Robot nie posiada enkoderów położenia głównych osi. Pozycja jest wyznaczana na podstawie liczników kroków silników. Jeżeli silnik zgubi kroki albo mechanika zostanie przestawiona ręcznie po uruchomieniu, sterownik nie wykryje automatycznie tej różnicy.

Z tego powodu po każdym restarcie należy:

1. ustawić ARM1 i ARM2 w pozycji `0°`,
2. wykonać procedurę HOME osi Z,
3. dopiero potem przejść do TEACH lub AUTO.

## Pliki źródłowe elektroniki

Dokumentacja płytki znajduje się w katalogu [`../PCB`](../PCB), a edytowalne pliki EasyEDA w [`../PCB/EasyEDA`](../PCB/EasyEDA).

## Pliki mechaniczne

Modele do druku 3D znajdują się w [`../3DPrinting`](../3DPrinting). Informacje o pochodzeniu modeli i licencjach są opisane w [`../3DPrinting/README.md`](../3DPrinting/README.md) oraz [`../THIRD_PARTY_NOTICES.md`](../THIRD_PARTY_NOTICES.md).
