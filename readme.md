# Materialverwaltung

Ein simpler webserver, welcher als Backend c hat.

## Kompilieren

Um das mit dem gcc Kompiler zu machen, einfach die folgenden Kommandos benutzen. (Ka wie das mit den anderen geht)

### Linux

`gcc webapp.c database.c filemanager.c linuxweb.c -o webapp -pthread -lm`

## Starten und Stoppen des Servers

Server starten mit den Kommandos oder `./webapp`<br>
Server stoppen, indem man entweder eine Anfrage auf die Route `/stopServer` macht, woraufhin bei der nächsten abarbeitung des callstacks das Program ordnungsgemäß gestoppt wird. Alternativ kann er auch einfach mit STRG + C gestoppt werde, wobei bei dieser Variante der Server an der aktuellen Stelle unterbrochen wird, und weder der Allocierte speicher befreit wird, noch die datenbank gespeichert wird.

## Programablauf

- Datenbank wird erstellt
- Die database.txt datei wird eingelesen und die Datenbank damit gefüllt
- Ein Socket wird im Betriebsystem registriert, auf welchem auf Anfrage abgehört wird (port 3333, kann aber beim Aufruf der Funktion selbst festgelegt werden)
- Wenn eine Anfrage kommt, wird diese auf einem Stack gespeichert, welcher mit `acceptClient` abgearbeitet wird. Dieser Funktion muss eine Buffergröße!!! und eine Callbackfunktion übergeben werden
- Anfrage wird angenommen und ein neuer Thread registriert, damit weitere anfragen parallel laufen können
- Ein Struct mit Methode, route und body wird erstelle und die Callbackfunktion damit aufgerufen
- Je nach anfrage werden dann die verschiedenen html dokumente geladen und weitere sachen damit gemacht
- Wenn die STOPSERVER variable 1 ist, wird vor der nächsten Abarbeitung des Stacks die While schleife unterbrochen und das program beginnt sich zu beenden
- Beim betriebssystem wird die Socket wieder freigegeben
- Für die komplette Datenbank wird ein String erstellt, welcher anschließend in eine Datei geschrieben wird
- Zuletzt wird der Allociert speicher für die Datenbank freigegeben

## Erklärung der einzelnen Teile

### webapp.c

Dies ist der einstiegspunkt des Programs welcher die main function beinhaltet. In dieser müssen alle Header der anderen Dateien included sein (in den dateien an sich ist dies nicht notwendig. Bsp.: wenn database.c keine deklarierungen von database.h benötigt, muss database.h nicht in database.c included sein, da database.h nur Funktionsprototypen bestitzt. Da aber in webapp.c alle funktionen der anderen Dateine benötigt werden, muss auch darin alle Header dateien included sein). Die Datei ist eigentlich nur dafür da, um die Funktionen der anderen Dateien aufzurufen und somit einen sauberen Überblick über die Funktionsweise zu ermöglichen. Wichtig ist, dass in dieser Datei das Callback deklariert werden muss, welches an jede Anfrage übergeben wird.

### database.c

Diese beinhaltet alle Funktionen, für welche die Datenbank benutzt wird. Die Datenbank ist eine simple einfach verkettete Liste von `Item` elementen. Hierbei ist das Startelement, welche an alle Funktionen als database pointer übergeben wird lediglich ein Element, mit den Werten NULL, wodurch der Anfang makiert wird. Wenn eine Funktion dann aufgerufen wird wird dieses Startelement übergeben, wodurch dank der next pointer durch die Liste iteriert werden kann.<br>
Wenn ein neues Element hinzugefügt wird direkt geschaut, an welche stelle es Alphabetisch sortiert hingehört, wodurch die Liste zu jeder Zeit alpahabetisch sortiert ist.<br>
Die CreateHTML methode ist eine besonder, da diese das Eingelesene datatable.html dokument mit den Werten der Datenbank füllt. Hierbei wird `§` als Platzhalter genommen.

### filemanager.c

Diese Datei beinhaltet zwei Funktionen zum Speichern und Lesen von Dateien.

### linuxweb.c

Diese Datei beinhaltet alle Funktionen, welche zum Betreiben eines Webservers vonnöten sind. Dies sind:
- Ein Socket registrieren
- Anfragen annehmen und diese Auf einen neuen Thread setzten
- Anfragen bearbeiten
- Die Anfragen zu parsen, damit die Daten leicht zu benutzen sind
- Antworten and den Clienten zu schicken (200 Success und 404 Not found)
- Ein Map datentyp zu parsen damit der mitgesendete Body des Clienten einfach als Key-Value paare abgearbeitet werden können
Die Funktionsweise der Registrierung eines Webservers wurde bereits oben erklärt

