# Betriebssysteme

Erwartete Funktionalität

Wechseln des Arbeitsverzeichnisses

Mit dem Befehl "cd" soll das aktuelle Arbeitsverzeichnis gewechselt werden. Dabei sollen relative und absolute Pfade verwendet werden können. Das schließt die Verwendung von ".." im Argument mit ein.

Hier gab es in den Abgaben gelegentlich Probleme, mit absoluten Pfaden, die über dem Startverzeichnis lagen. Ebenfalls gab es manchmal Probleme bei Befehlen wie "cd ../a/b", bei dem zuerst in das Elternverzeichnis, des aktuellen Arbeitsverzeichnisses gewechselt werden soll und von dort aus in den Unterordner b.



Prompt

Die Ausgabe des aktuellen Arbeitsverzeichnis, relativ zum Einstiegspunkt wurde bereits hier ausführlich erläutert. 

Häufige Fehler waren, dass immer der gesamte Pfad des Arbeitsverzeichnisses ausgegeben oder nur die Eingaben des cd-Befehls hintereinander gehangen wurden.



Ausführen eines Programms

Die Shell sollte in der Lage sein Programme zu starten, direkt oder im Hintergrund, wenn ein "&" übergeben wurde. Wenn ein "&" verwendet wurde, sollte die Process-ID (pid) ausgegeben werden. Es sollten für das Programm Parameter verwendet werden können. 

Hier gab es häufiger Probleme beim Übergeben der Parameter. Es sollten weder das "&", noch das aktuelle Arbeitsverzeichnis, als Parameter an das Programm übergeben werden. 
Es konnte bei der Implementation auf verschiedene exec-Varianten zurückgegriffen werden, wodurch manche Abgabe nur Programme aus dem aktuellen Arbeitsverzeichnis starten konnten. Das ist in Ordnung, wünschenswert wäre dennoch die Verwendung von "execvp".



Warten auf Prozesse

Mit dem Befehl "wait" und der Angabe der pid(s), sollte auf Prozesse gewartet werden. Dabei sollte solange wie gewartet wird, kein neuer Befehl auf der Shell ausführbar sein. Ist der Prozess terminiert sollten möglichst viele Informationen ausgegeben werden. Es sollte mindestens der Rückgabewert angezeigt werden. 
Das Warten war unterbrechbar zu gestalten, d.h. mit strg+c sollte das Warten beendet werden. Sowohl die Shell, als auch gestartete Prozesse, sollten danach weiter ausgeführt werden.

Ein häufiger Fehler war die Ausgabe falscher Statusmeldungen. Wenn wait für mehrere pids ausgeführt wird, sollte der Rückgabewert auch jeweils von dem beendeten Prozess stammen. Ein weiterer Fehler war, dass das Unterbrechen mit strg+c die Shell beendete und/oder den Prozess auf den gewartet wurde.



Pipe

Mit der Pipe soll die Ausgabe auf stdout des ersten Prozesses, nach stdin des zweiten Prozesses umgeleitet werden. Dessen Ausgabe wiederum, sollte auf der Shell erscheinen. Jedes Programm sollte mit Parametern versehen werden können.

Ein häufiger Fehler war, dass die Ausgabe des zweiten Prozesses nicht auf der Shell erschienen ist. 
Da die Umsetzung eine kompliziertere Aufgabe war, gab es viele Implementationen die nicht richtig funktioniert haben. Um zu testen, ob eure Shell das richtige tut, empfiehlt sich gegen eine echte Shell eurer Wahl zu testen. Die Ausgaben sollte identisch sein. 
Solltet ihr eine exec-Variante genutzt haben, die nicht die Umgebungsvariablen verwendet und die Ausgabe des zweiten Prozesses nicht auf die Shell umleiten, wird die Korrektur etwas schwieriger für mich.


Mit folgenden Befehlen solltet ihr testen können, ob ihr die Pipe richtig umgesetzt habt.

Wenn ihr die Ausgabe auf die Shell zurückleitet: 

ps auxwww | grep fred


Wenn ihr die Ausgabe nicht auf die Shell zurückleitet:

ifconfig | tee aaaaa.txt
Der Befehl schreibt die Ausgabe von ifconfig in eine Datei "aaaa.txt" im aktuellen Arbeitsverzeichnis.



Exit

Der Befehl sollte die Shell beenden.

