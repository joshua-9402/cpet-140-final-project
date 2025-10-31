// MATANGUIHAN, JANN VINCENT A.

/*
 * Issues:
 *  Save the products after the program exits
 *  the entire code runs on a single loop making it inefficient
 *  GUI is required
 *  there is no application of functions and classes here
 *  must be cross-platform (optional)
 */

#include <iostream>
#include <iomanip>
using namespace std;

struct Product {
    string name;
    double price;
    int quantity;
};

void main_GUI() {
    const int MAX_PRODUCT = 10; // ket ilan pede, basta realistic. so bawal 100+
    Product products[MAX_PRODUCT];
    int productCount = 0;
    int choice;
    
    do { // line 18 - 28 is yung parang interface
        cout << "================================================" << endl;
        cout << "     SIMPLE INVENTORY SYSTEM FINAL PROJECT" << endl;
        cout << "================================================" << endl;
        cout << "1. Add Product" << endl;
        cout << "2. View Products" << endl;
        cout << "3. Update Product/s" << endl;
        cout << "4. Delete Product/s" << endl;
        cout << "5. Exit" << endl;
        cout << "================================================" << endl;
        cout << "Enter Choice: ";
        cin >> choice;
        cout << endl;
            
        if (choice == 1) { // dito kayo maglalagay ng product. name of product, price, at kung ilan man to. (to ung C sa CRUD. ung Create)
            if (productCount >= MAX_PRODUCT) {
                cout << "Inventory FULL!" << endl; 
                cout << " " << endl;
            } else {
                cout << "Enter Product Name: ";
                cin.ignore();
                getline(cin, products[productCount].name);
                    
                cout << "Enter Price: P"; // pa change nlng po ng "P" into peso sign, d lng po ako maalam sa laptop hehe
                cin >> products[productCount].price;
                    
                cout << "Enter quantity: ";
                cin >> products[productCount].quantity;
                
                cout << "Product has been successfully added!" << endl;
                productCount++;
                cout << " " << endl;
            }
        } else if (choice == 2) { // pede mo makita ung mga product na i-nadd nyo(ito ung R sa CRUC or ung Read)
            if (productCount == 0) {
                cout << "No Available Stock!" << endl;
                cout << " " << endl;
            } else {
                cout << "================================================" << endl;
                cout << left << setw(20) << "Product"
                     << setw(10) << "Price"
                     << setw(10) << "Quantity" << endl;
                cout << "================================================" << endl;
                    
                double totalValue = 0;
                for (int i = 0; i < productCount; i++) { // ito ung pinaka table na makikita pag nag view kayo
                    cout << left << setw(20) << products[i].name
                         << setw(10) << fixed << setprecision(2) << products[i].price
                         << setw(10) << products[i].quantity << endl;
                    totalValue += products[i].price * products[i].quantity;
                }
                cout << "================================================" << endl;
                cout << "Total Inventory Value P" << fixed << setprecision(2) << totalValue << endl; // papalit nlngdin po ng "P" dito into peso sign
                cout << " " << endl;
            }
        }
        if (choice == 3) { //dito ilalagay ung U sa CRUD or ung update
            cout << "This functionn have yet to be added." << endl;
            cout << " " << endl;
        }
        if (choice == 4) { // dito nmn ung sa D ng CRUD or ung delete
            cout << "This functionn have yet to be added." << endl;
            cout << " " << endl;
        }

    } while (choice != 5);

    return 0;
}