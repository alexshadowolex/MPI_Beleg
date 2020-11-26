# Parallele Suche nach Bildaehnlichkeiten in Bildsequenzen

Eine MPI-Prozessgruppe soll kooperativ, arbeitsteilend nach Aehnlichkeiten in Bildern suchen und damit die Daten
fuer eine Differenzkodierung 
fuer Bilder bereitstellen. Als Anwendungsfall sollen einzelne Bildpaare, bzw. Sequenzen der Differenzkodierung unterzogen werden, z.B. Foto-Serienaufnahmen,
die so speicherplatzsparend kodiert werden koennen. 
Fuer einen Bildbereich sind die am staerksten aehnlichen Bildanteile zu einem
Vorgaengerbild (bzw. ein vorab bekanntes Referenzbild) zu suchen. 
Der Bildbereich kann dann durch einen Verschiebungsvektor und ein Differenzbild zum
Vorgaengerbild speicherplatzsparend kodiert werden. 
Die Suche der aehnlichen Bildanteile erfolgt durch systematisches Ausprobieren
moeglicher Verschiebungen, der Berechnung der Bilddifferenz 
(Summe absoluter Differenzen der Pixelwerte, SAD) und der anschliessenden Auswahl eines Verschiebungsvektors. 
Diese Schritte sind rechenaufwaendig und sollen parallelisiert werden. 

Funktionale Ziele:
* 	Einlesen der Bilder, z.B. mittes stb-lib und geeignetes Verteilen der Daten
* 	Berechnung von Veschiebungsvektoren und Differenzbildern fuer Bildpaare
*	es wird keine vollstaendige Bildkodierung verlangt 


Ziele bzgl. Verteilung/Parallelisierung:
* 	Identifikation von Potenzial fuer eine Parallelisierung: Berechnung SAD, parallele Berechnung fuer verschiedene Verschiebungsvektoren, 
	oder parallele Berechnung fuer verschiedene Bilder? 
* 	Welcher Gewinn entsteht durch Nutzung mehrerer MPI-Prozesse (Anzahl 1,2,3,4,5,...)?
*	Welcher Unterschied besteht zwischen knotenlokalen Prozessgruppen (alle MPI Prozesse auf einem Rechner) und Prozessgruppen, die ueber mehrere Rechner verteilt sind?
Programmiersprachen: C, C++, MPI, (wahlweise MPI-Threads), stb-lib (zum lesen von jpg-Bilddateien in C-Arrays)



### Hinweise:
* X11-Server, putty, forwarding zu x11-bildschirmausgaben (ssh -x11)
  -> XMING
* Wie suche nach ähnlichem Bildbereich stattfindet? BSP: Gesicht um 5 px nach rechts und 3 nach unten verschoben: wie finden? Bereich darum zu erst (nahe verschiebungsvektoren)
* Testdaten: zwei leicht unterschiedliche bilder mit paint (32x32) erstellen
* Helligkeitswert aus rot, grün, blau und damit differenz bilden
* current min sad merken
* alle neuen sads, die größer sind, aus der auswahl entfernen
* bereich um makroblock zum suchen -> wert einfach per command line festlegen