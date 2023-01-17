# Scopo

L'applicazione in oggetto deve occuparsi di gestire l'esecuzione e l'aggiornamento di un altro eseguibile.

L'ambiente di esecuzione sara' Linux; a parte cio' che viene compilato all'interno del progetto non si possono fare grosse assunzioni sulle librerie disponibili (tra queste, sicuramente `math` e `pthreads`).

Da qui in avanti si fara' riferimento all'eseguibile risultato dal progetto in oggetto come "supervisore".
Il supervisore controlla un altro eseguibile, da qui in avanti indicato come "applicazione".

Il contesto finale di utilizzo del supervisore e' quello di una interfaccia utente dotata di display.
Non si fanno assunzioni sul funzionamento dell'applicazione, e l'unica interazione che il supervisore ha con essa e' tramite il valore di ritorno dell'eseguibile.

Il supervisore deve prendere il controllo del display in alcuni contesti, lasciandolo all'applicazione durante il funzionamento normale.

L'interfaccia grafica del supervisore e' costruita tramite LVGL.

## Funzioni:

Il supervisore deve lanciare e monitorare lo stato dell'applicazione; in particolare:

 - Dovra' contenere un menu' (da qui in avanti noto come "impostazioni") dal quale sara' possibile scegliere le seguenti funzioni:
    - Caricare una nuova versione dell'applicazione
    - Esportare file di diagnosi dell'applicazione (log, core dump, etc...)
    - Lanciare l'applicazione
    - Visualizzare la versione del supervisore
    - Visualizzare la versione dell'applicazione

 - Se l'applicazione (sotto forma di eseguibile autonomo) non viene trovata sul filesystem il supervisore deve entrare nelle impostazioni
 - Se l'applicazione termina con successo (codice 0) il supervisore deve lanciarne un'altra istanza
 - Se l'applicazione termina con successo ripetutamente in un breve periodo di tempo il supervisore deve mostrare una schermata in cui avvisa l'utente e permette di visualizzare le impostazioni
 - Se l'applicazione termina con un codice di errore il supervisore deve mostrare una schermata in cui avvisa l'utente e permette di visualizzare le impostazioni
 - Alla partenza il supervisore deve presentare una schermata anonima (e.g. nera) per alcuni secondi con un tasto nascosto (e.g. 100x100 in alto a destra) che se premuto interrompe il caricamento dell'applicazione e permette di entrare nelle impostazioni

## Parametri

Il supervisore deve essere modificabile nei seguenti parametri a tempo di compilazione (e.g. macro):

 - Risoluzione dell'interfaccia grafica

Il supervisore deve essere modificabile nei seguenti parametri a tempo di esecuzione tramite flag:

 - Percorso dell'applicazione
 - Percorsi dei file di diagnostica dell'applicazione
 - Tempo e numero di tentativi prima di entrare nelle impostazioni

Il supervisore comunica con l'applicazione soltanto in due modi:

 1. Tramite il codice di chiusura dell'applicazione
 2. Passando il flag `-v` all'applicazione e aspettandosi dall'esecuzione risultante la sola stampa della versione dell'applicativo. Sarebbe opportuno anche inserire un timeout per evitare che l'applicazione prosegua anziche' stampare la versione e fermarsi.

 