Priklady Aero pre Feedback Lab (FBL), hardver Aero R1

Prenesene z AutomationShield/examples/AeroShield. Povodne autorstvo a licencia
CC BY-NC 4.0 su uvedene v jednotlivych prikladoch; MPC a APRBS data zachovavaju
povodne hlavicky. AutomationShield nie je potrebny na kompilaciu.

OpenLoop       - potenciometer POT1 nastavuje vstup motora 0 az 3 V.
PID            - PID regulacia uhla, automaticka trajektoria alebo MANUAL = 1.
LQI            - povodne LQI zisky, rychlost vypocitana diferenciou uhla.
EMPC           - explicitne MPC, matice pre Uno/Mega 2560/Due podla dosky.
Identification - povodny APRBS experiment; pre validaciu pouzi aprbsU_val.h.

Postup:
1. Otvor prislusny .ino subor v Arduino IDE a vyber dosku.
2. Nahraj program a otvor Serial Monitor alebo Serial Plotter, 115200 baud.
3. Pri kalibracii nechaj rameno v pokojovej nulovej polohe.
4. Po troch pipnutiach blika zelena LED: caka sa na tlacidlo START.
5. START spusti experiment, USER ho zastavi. Novy beh sa spusta resetom.

LED indikacia:
- Modra: inicializacia/kalibracia, pripravenost alebo ukonceny experiment.
- Blikajuca zelena po kalibracii: cakanie na START.
- Zelena: experiment bezi.
- Cervena: chyba kalibracie, prekroceny uhol, nestihnuta vzorkovacia perioda
  alebo stav mimo oblasti MPC. Motor sa zastavi; dovod je na seriovej linke.
- Cervene blikanie pri inicializacii: chyba snimaca alebo magnetu.
- Trvala cervena pri inicializacii: nespravne nastavena verzia hardveru.

FBL cita uhol v radianoch a writeInput() prijima 0 az 3 V s linearnym
prevodom na PWM. Priklady prevadzaju uhol okolo nuly na interval -pi az pi.
PID zachovava povodne percenta uhla (100 % = 90 stupnov) a prevadza
vystupne percenta na 0 az 3 V. Serial Plotter vzdy zobrazuje skutocny
obmedzeny prikaz vstupu vo voltoch, nie povodny prikaz pred obmedzenim.

Povodny AutomationShield pouzival iny prevod napatie/PWM (RMS, 3.7 V).
Povodne PID/LQI zisky, MPC model a APRBS data preto nie su novou identifikaciou
FBL zariadenia. Pred pouzitim LQI/MPC treba overit model a zisky na hardveri;
obmedzenie prikazu MPC na 3 V nezarucuje splnenie povodnych predikcii modelu.
Volitelna vetva Kalman z AutomationShield nie je prenesena; LQI a MPC
pouzivaju priamo merany uhol a diferenciu pre rychlost, ako povodny default.

Vzorkovanie pouziva spolocny modul FBLSampling.h z FBL.
Na Uno/Mega/Due pouziva hardverovy timer, na ostatnych doskach micros().
Prerusenie len oznami novu vzorku; riadenie a obsluha chyby bezia v loop().
Oneskorenie celej periody alebo prekrocenie terminu pocas kroku zastavi motor
pri najblizsom spracovani Sampling.update(). Podrobnosti: ../Sampling/README.txt.
PID ma periodu 5 ms, ostatne priklady 10 ms. Riadiace priklady vypisuju data
kazdych 20 ms, Identification kazdych 10 ms.

Potenciometer predpoklada predvolene 10-bitove analogRead() (0 az 1023)
a motor predvolene 8-bitove analogWrite() (0 az 255), rovnako ako Aero API.
Chyby komunikacie pocas citania uhla momentalne Aero.readSensor() nehlasi:
LED chyby snimaca sa vztahuje na kontrolu pri inicializacii.
