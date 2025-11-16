#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

int main() {
    int n;
    cin >> n;

    vector<string> books(n);
    for (int i = 0; i < n; i++) {
        cin >> books[i];
    }

    int q;
    cin >> q;

    while (q--) {
        string operation;
        cin >> operation;

        if (operation == "CARI") {
            string code;
            cin >> code;
            
            bool found = false;
            for (int i = 0; i < n; i++) {
                if (books[i] == code) { // Linear search [cite: 204]
                    found = true;
                    break;
                }
            }
            cout << (found ? "ADA" : "TIDAK ADA") << endl; 

        } else if (operation == "HITUNG") {
            string substring;
            cin >> substring;
            
            int count = 0;
            for (int i = 0; i < n; i++) {
                if (books[i].find(substring) != string::npos) {
                    count++;
                }
            }
            cout << count << endl; 

        } else if (operation == "URUT") {
           
            vector<string> sorted_books = books; 
            sort(sorted_books.begin(), sorted_books.end()); 
            
            for (const string& book : sorted_books) {
                cout << book << endl; 
            }
        }
    }

    return 0;
}