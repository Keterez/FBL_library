Sampling pre Feedback Lab

Pouzitie:

  #include <FBLSampling.h>

  void step() {
    // Jedna vzorka: citanie snimaca, vypocet regulatora, zapis vstupu.
  }

  void timingError() {
    // Sampling je uz zastaveny. Tu vypni motor a indikuj chybu.
  }

  void setup() {
    // Najprv inicializacia zariadenia a kalibracia.
    if (!Sampling.begin(5000, step, timingError)) {
      // Neplatna konfiguracia; dovod vrati Sampling.error().
    }
  }

  void loop() {
    Sampling.update();
  }

5000 znamena periodu 5000 mikrosekund = 5 ms = 200 Hz. Prvy krok nastane
po jednej celej periode. begin() vracia bool, update() vracia true, ak
vyvolal krok. Ziadne dynamicke alokacie ani povinne pomocne flagy v sketche.
Nazov FBLSampling.h umoznuje jednoznacny vyber FBL aj pri nainstalovanej
AutomationShield, ktora ma vlastny nekompatibilny Sampling.h.

Rezimy (posledny, nepovinny argument begin):
- SamplingMode::Auto: hardverovy timer na Uno Rev3, Mega 2560 a Due;
  na ostatnych Arduino doskach softverove casovanie cez micros().
- SamplingMode::Hardware: vyzaduje podporovany timer. Pri nedostupnosti,
  obsadenom timeri alebo nepodporovanej periode vrati begin() false.
- SamplingMode::Software: casovanie cez micros(), bez spustenia timeru.

ISR iba zaznamena novu vzorku. step() aj timingError() sa vykonavaju
v Sampling.update(), s beznym kontextom hlavnej slucky. Mozu pouzit Wire
a Serial. update() treba volat casto; ani hardverovy timer neodstrani
oneskorenie sposobene inou pracou v loop(). Nie je to hard real-time system.
begin(), stop() a update() volaj z hlavnej slucky so zapnutymi preruseniami.

Terminy zostavaju na povodnej casovej mriezke. Kratke oneskorenie neposunie
nasledujuce vzorky. Ak caka viac vzoriek, sampling sa zastavi pred dalsim
krokom; stare vzorky sa nedohanaju opakovanym volanim regulatora. Ak krok
dosiahne alebo prekroci termin dalsej vzorky, sampling sa zastavi po kroku.
V oboch pripadoch sa nastavi SamplingError::Overrun a raz sa zavola obsluha
chyby. Bez nej sa sampling aj tak zastavi; aplikacia zodpoveda za vypnutie
aktuatora. Pri zablokovanom loop() sa chyba aj vypnutie motora vykonaju az
po navrate do update(). Dlhe vypnutie preruseni moze stratit hardverove tiky
a narusit aj samotne micros().

Sampling.stop() zastavi timer aj dalsie vzorky. Moze sa volat v step().
Normalne zastavenie nevyvola timingError(). Novy begin() mimo callbackov
resetuje chybu a statistiky a spusti novu casovu mriezku. begin() v callbacku
vrati false bez zmeny stavu. Rekurzivny update() nevyvola dalsi krok.
Neplatna konfiguracia v begin() sampling zastavi a nastavi error(); obsluha
timingError() sa pre chyby konfiguracie nevola.

Diagnostika:
- running(), error(), mode(): aktualny stav, posledna chyba a zvoleny rezim.
- periodUs(), periodSeconds(): perioda v mikrosekundach alebo sekundach.
- sampleCount(): pocet vykonanych krokov od begin(), aj zastavujuci krok.
- lastExecutionUs(), maxExecutionUs(): trvanie posledneho/najdlhsieho kroku.
- lastLatenessUs(): oneskorenie zaciatku posledneho kroku oproti mriezke.
  Trvanie zahrna obsluhu preruseni pocas kroku, nie chybovy callback.
  Pocitadla su uint32_t a po preteceni pokracuju od nuly.

Timery a obmedzenia:
- Uno Rev3: Timer1, ovplyvni PWM na pinoch 9 a 10.
- Mega 2560: Timer5, ovplyvni PWM na pinoch 44, 45 a 46.
- Due: TC1 kanal 2 (TC5).
- Timer musi byt vyhradeny pre Sampling. Servo alebo ina kniznica nesmie
  pouzivat ten isty timer/ISR. Povodna konfiguracia timeru sa neobnovuje.
- Jeden hardverovy timer moze vlastnit len jedna instancia SamplingClass;
  druha dostane TimerBusy. Softverovych instancii moze byt viac.
- V softverovom rezime sa timer nespusta. Ak ina kniznica definuje rovnaky
  ISR, treba kompilovat celu FBL s -DFBL_SAMPLING_SOFTWARE_ONLY, cim sa
  hardverove backendy aj ich ISR vylucia. Samotne #define v .ino nestaci.
- AVR: zvoli sa najmensia delicka z 1/8/64/256/1024, ktora umozni presnu
  periodu v 16-bitovom timeri. Nepresna alebo prilis dlha perioda sa odmietne.
  Pri 16 MHz je maximum 4194304 us; napr. 5000 a 10000 us su presne.
- Due: timer bezi na MCK/2; perioda musi byt presne reprezentovatelna
  32-bitovym poctom tikov (pri 84 MHz maximalne 102261126 us).
- Software: perioda 1..2147483647 us; presnost je dana micros() a frekvenciou
  update(). Na Uno ma micros() rozlisenie 4 us, nie 1 us.
- Pretecenie 32-bitoveho micros() je osetrene. Slucka nesmie zostat bez
  obsluhy dlhsie ako INT32_MAX us (priblizne 35 minut).

Minimalny priklad: Periodic/Periodic.ino.
Aero priklady maju chybovy callback s vypnutim motora a cervenou LED.
Deterministicke C++ testy na PC: python tests/sampling/run_tests.py
(MSVC C++ Build Tools vo Windows alebo C++11 kompilator na Linux/macOS).
