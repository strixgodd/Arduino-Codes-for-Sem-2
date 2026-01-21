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

void mergeSort(int arr[], int l, int r){//merge algo
  if (l < r) {
    int m = l + (r - l) / 2;
    mergeSort(arr, l, m);
    mergeSort(arr, m + 1, r);
    merge(arr, l, m, r);
  }
}

const byte START1 = 0xAA; 
const byte START2 = 0x55; 

int arr[] = {6,5,4,3,2,1,9,17,15};// og array
const int n = sizeof(arr) / sizeof(arr[0]);// og array size


int right[n/2 + 1];
int left[n/2 + 1];

// size_left will be bigger/equal when n is odd
const int size_left  = (n + 1) / 2; // eg n=7 -> 4
const int size_right = n / 2;// eg n=7 -> 3

int right_recv[56];// buffer for receiving sorted right from slave

bool sent = false;
bool gotBack = false;

void setup() {
  Serial.begin(9600);   // Serial Monitor debugging
  Serial1.begin(9600);  // UART Master<->Slave (TX1/RX1)

  // SPLIT OG ARRAY
  // Left part takes extra element if n is odd
  for (int i = 0; i < size_left; i++) {
    left[i] = arr[i];
  }
  // Right part takes remaining elements
  for (int i = 0; i < size_right; i++) {
    right[i] = arr[size_left + i];
  }
}

void loop() {

  // SEND RIGHT PART TO REINFIELD
  if (!sent) {

    // header start
    Serial1.write(START1);
    Serial1.write(START2);

    // send the number of elements in RIGHT (int => 2 bytes on Mega)
    Serial1.write((byte*)&size_right, sizeof(size_right));// the value of size_right in little endian format is sent to slave

    // send the raw bytes of the right array only (size_right elements)
    Serial1.write((byte*)right, size_right * sizeof(int));

    Serial.println("Master: Sent RIGHT part to Slave");

    //2) SORT LEFT PART LOCALLY 
    mergeSort(left, 0, size_left - 1);

    Serial.print("Master: Sorted LEFT = ");
    for (int i = 0; i < size_left; i++) {
      Serial.print(left[i]);
      Serial.print(" ");
    }
    Serial.println();

    sent = true;
  }

  // 3) RECEIVE SORTED RIGHT BACK 
  if (sent && !gotBack) {

    // check if header arrived
    if (Serial1.available() >= 2) {
      if (Serial1.read() == START1 && Serial1.read() == START2) {

        // receive size of array (2 bytes)
        int recv_size = 0;
        while (Serial1.available() < (int)sizeof(recv_size)) {}
        Serial1.readBytes((char*)&recv_size, sizeof(recv_size));

        // sanity check
        if (recv_size > 56) {
          Serial.println("Master ERROR: received size too large!");
          return;
        }

        // now receive recv_size integers
        int totalBytes = recv_size * sizeof(int);
        while (Serial1.available() < totalBytes) {}
        Serial1.readBytes((char*)right_recv, totalBytes);

        Serial.print("Master: Received sorted RIGHT = ");
        for (int i = 0; i < recv_size; i++) {
          Serial.print(right_recv[i]);
          Serial.print(" ");
        }
        Serial.println();

        gotBack = true;

        // 4) BUILD FINAL ARRAY INTO arr[] 
        // Put sorted left into first part of arr[]
        for (int i = 0; i < size_left; i++) {
          arr[i] = left[i];
        }

        // Put sorted right into second part of arr[]
        for (int i = 0; i < recv_size; i++) {
          arr[size_left + i] = right_recv[i];
        }

        // Now merge the two sorted halves inside arr[] itself
        merge(arr, 0, size_left - 1, n - 1);

        Serial.print("Master: FINAL SORTED ARRAY = ");
        for (int i = 0; i < n; i++) {
          Serial.print(arr[i]);
          Serial.print(" ");
        }
        Serial.println();

        while (1); // stop forever (only once)
      }
    }
  }
}
