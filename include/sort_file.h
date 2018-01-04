#ifndef SORT_FILE_H
#define SORT_FILE_H

// Code for sorted files //
extern char code[14];

typedef enum SR_ErrorCode {
  SR_OK,
  SR_ERROR
} SR_ErrorCode;

typedef struct Record {
	int id;
	char name[15];
	char surname[20];
	char city[20];
} Record;

// Initialize sort file //
SR_ErrorCode SR_Init();

// Create and initialize empty sort file //
SR_ErrorCode SR_CreateFile(const char*);

// Open given sort file. Return fileDesc of file //
SR_ErrorCode SR_OpenFile(const char*,int*);

// Close given sort file //
SR_ErrorCode SR_CloseFile(int);

// Add new record in sort file. New record is added at the end of the file //
SR_ErrorCode SR_InsertEntry(int,Record);

/*
 * Η συνάρτηση αυτή ταξινομεί ένα BF αρχείο με όνομα input_​fileName ως προς το
 * πεδίο που προσδιορίζεται από το fieldNo χρησιμοποιώντας bufferSize block
 * μνήμης. Το​ fieldNo είναι ο αύξων αριθμός του πεδίου, δηλαδή αν το
 * fieldNo = 0, τότε το πεδίο ως προς το οποίο θέλουμε να κάνουμε ταξινόμηση
 * είναι το id, αν fieldNo = 1, τότε το πεδίο ως προς το οποίο θέλουμε να
 * ταξινομήσουμε είναι το name, κτλ. Η συνάρτηση επιστρέφει SR_OK σε περίπτωση
 * επιτυχίας, ενώ σε διαφορετική περίπτωση κάποιος κωδικός λάθους.
 * Πιο συγκεκριμένα, η λειτουργικότητα που πρέπει να υλοποιηθεί είναι η ακόλουθη:
 *
 *    * Να διαβάζονται οι εγγραφές από το αρχείο input_filename και να
 *      εισάγονται στο νέο αρχείο ταξινομημένες ως προς το πεδίο με αρίθμηση
 *      fieldNo. Η ταξινόμηση θα γίνει με βάση τον αλγόριθμο εξωτερικής
 *      ταξινόμησης (external sort).
 *
 *    * Ο αλγόριθμός θα πρέπει να εκμεταλλεύεται όλα τα block μνήμης που σας
 *      δίνονται από την μεταβλητή bufferSize και μόνον αυτά. Αν αυτά τα block
 *      είναι περισσότερα από BF_BUFFER_SIZE ή μικρότερα από 3 τότε θα
 *      επιστρέφεται κωδικός λάθους.
 */
SR_ErrorCode SR_SortedFile(
  const char* input_filename,   /* όνομα αρχείου προς ταξινόμηση */
  const char* output_filename,  /* όνομα του τελικού ταξινομημένου αρχείου */
  int fieldNo,                  /* αύξων αριθμός πεδίου προς ταξινόμηση */
  int bufferSize            /* Το πλήθος των block μνήμης που έχετε διαθέσιμα */
  );

// Print all records in sort file //
SR_ErrorCode SR_PrintAllEntries(int);

#endif // SORT_FILE_H
