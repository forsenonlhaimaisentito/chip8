# Emulatore ed assembler/disassembler in C
## Con commenti in italiano

### Compilazione
    git clone https://github.com/forsenonlhaimaisentito/chip8.git
	cd chip8
	autoreconf -i
	./configure && make
	
### Utilizzo
Il progetto comprende due programmi: **c8emu** e **c8as**,
rispettivamente emulatore ed assembler/disassembler.

#### c8emu
L'utilizzo è piuttosto semplice:

`./c8emu NOME_FILE COLORE SFONDO`

i colori primo piano e sfondo sono *opzionali* e vanno
specificati in esadecimale, come la notazione HTML, esempio:

`./c8emu ../PONG 00FF00 000000`

esegue il programma `../PONG` e disegna verde su nero.

#### c8as
Prende uno o due argomenti, nel caso di un argomento,
effettua una traduzione da codice macchina a mnemonico;
se invece ha due argomenti, traduce il file del primo in
codice macchina e scrive il risultato nel secondo, ad esempio
se voglio compilare il file `sorgente.txt` in `programma`, farò:

`./c8as sorgente.txt programma`

mentre per vedere il sorgente di un programma `programma`:

`./c8as programma`

#### Sintassi assembler
La sintassi dell'assembler ricorda quelle di molti altri,
ha funzionalità come label, db e resb, mentre riconosce le seguenti
istruzioni:

* CLS
* RET
* JP
* CALL
* SKIPE
* SKIPNE
* LD
* ADD
* SUB
* RSB
* OR
* AND
* XOR
* SHR
* SHL
* RAND
* DRAW
* SKIPDW
* SKIPUP
* IN
* SPRITE
* BCD
* STOR
* LOAD
* DB
* RESB

Sezione da completare.
