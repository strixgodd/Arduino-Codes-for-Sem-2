// #define MAX 3
// int arr[MAX];

void merge(int arr[], int l, int m, int r) {
  int i, j, k;
  int n1 = m - l + 1;
  int n2 = r - m;

  int L[n1];
  int R[n2];

  for (i = 0; i < n1; i++)
    L[i] = arr[l + i];

  for (j = 0; j < n2; j++)
    R[j] = arr[m + 1 + j];

  i = 0;
  j = 0;
  k = l;

  while (i < n1 && j < n2) {
    if (L[i] <= R[j])
      arr[k++] = L[i++];
    else
      arr[k++] = R[j++];
  }

  while (i < n1)
    arr[k++] = L[i++];

  while (j < n2)
    arr[k++] = R[j++];
}
void mergeSort(int arr[], int l, int r) {
  if (l < r) {
    int m = l + (r - l) / 2;

    mergeSort(arr, l, m);
    mergeSort(arr, m + 1, r);

    merge(arr, l, m, r);
  }
}

const byte START1 = 0xAA;
const byte START2 = 0x55;

int arr[50];   // receiver buffer (make sure it's large enough)
int n = 0;

void setup() {//runs only once after the board powers on/reset
  Serial.begin(9600);    // Serial Monitor output
  Serial1.begin(9600);    // UART link from sender Mega
}

void loop() {
  //RECIEVING RIGHT HALF OF THE ARRAY

  // Step 1: Wait for header
  if (Serial1.available()>=2){//Serial1.available() Get the number of bytes (characters) available for reading from the serial port. This refers to data that has already been received and is currently stored in the serial receive buffer (which holds 64 bytes). The function returns the number of bytes available to read.
    if (Serial1.read() == START1 && Serial1.read() == START2){//Serial1.read() reads incoming serial data and consumes it from the serial buffer.The function returns the first byte of incoming serial data available (or -1 if no data is available).

      // Step 2: Wait for length bytes
      while (Serial1.available()<(int)sizeof(n));// busy wait until 2 bytes appear in the serial buffer

      Serial1.readBytes((char*)&n, sizeof(n));//n stores the size of the array,sending from master.

      // Safety check
      if (n > 50) {
        Serial.println("Error: received length too large!");
        return;
      }

      // Step 3: Wait for full array data
      int totalBytes = n * sizeof(int);
      while (Serial1.available() < totalBytes) {}

      Serial1.readBytes((char*)arr, totalBytes);

      // Print received array
      Serial.print("Received array of size ");
      Serial.println(n);

      // SORTING RIGHT HALF
      mergeSort(arr,0,n-1);
      for (int i = 0; i < n; i++) {
        Serial.print(arr[i]);
        Serial.print(" ");
      }
      Serial.println();

      //SEND BACK TO DRACULA

      // ---- SEND BACK to Master ----
      Serial1.write(START1);
      Serial1.write(START2);

      Serial1.write((byte*)&n, sizeof(n));
      Serial1.write((byte*)arr, totalBytes);

      Serial.println("Slave sent sorted array back");

      while (1); // stop forever after receiving once
    }
  }
}
