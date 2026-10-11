#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <regex>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <hpdf.h>
using namespace std;

const int MAX_USERS = 100;
const int MAX_CHARITIES = 50;
const int MAX_DONATIONS_PER_USER = 100;

struct Date {
    string date;
    string time;
};

struct Donation {
    int donationID = 0;
    int charityID = 0;
    double amount = 0.0;
    Date d;
    string message;
};

struct Charity {
    int charityID = 0;
    string name;
    string description;
    double targetAmount = 0.0;
    double currentAmount = 0.0;
    Date creationDate;
    string status;
};

struct Client {
    int userID = 0;
    string firstName;
    string lastName;
    string password;
    string phone;
    string email;
    int nbDonations = 0;
    Donation donations[MAX_DONATIONS_PER_USER];
    bool isAdmin = false;
};

Client users[MAX_USERS];
Charity charities[MAX_CHARITIES];
int userCount = 0;
int charityCount = 0;

string hashPassword(const string& password) {
    unsigned char digest[SHA256_DIGEST_LENGTH];
    EVP_MD_CTX* context = EVP_MD_CTX_new();

    if (context == nullptr) {
        throw runtime_error("Failed to create EVP_MD_CTX");
    }

    if (EVP_DigestInit_ex(context, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(context);
        throw runtime_error("EVP_DigestInit_ex failed");
    }

    if (EVP_DigestUpdate(context, password.c_str(), password.length()) != 1) {
        EVP_MD_CTX_free(context);
        throw runtime_error("EVP_DigestUpdate failed");
    }

    if (EVP_DigestFinal_ex(context, digest, nullptr) != 1) {
        EVP_MD_CTX_free(context);
        throw runtime_error("EVP_DigestFinal_ex failed");
    }

    EVP_MD_CTX_free(context);

    ostringstream oss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        oss << setw(2) << setfill('0') << hex << (int)digest[i];
    }
    return oss.str();
}

bool isValidEmail(const string& email) {
    const regex emailRegex("^[a-zA-Z0-9_+&*-]+(?:\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,7}$");
    return regex_match(email, emailRegex);
}

bool isValidPhone(const string& phone) {
    const regex phoneRegex("^[0-9]{8}$");
    return regex_match(phone, phoneRegex);
}

int generateUserID(bool isAdmin = false) {
    static bool seeded = false;
    if (!seeded) {
        srand(static_cast<unsigned>(time(0)));
        seeded = true;
    }

    if (isAdmin) {
        return (rand() % 10) + 1;
    }
    else {
        time_t now = time(0);
        return static_cast<int>(static_cast<unsigned>(now)) + (rand() % 1000);
    }
}

Date getCurrentDateTime() {
    time_t t = time(0);
    tm now;
    if (localtime_s(&now, &t) != 0) {
        throw runtime_error("Failed to get local time");
    }

    stringstream dateStream, timeStream;
    dateStream << setw(2) << setfill('0') << now.tm_mday << "-"
        << setw(2) << setfill('0') << 1 + now.tm_mon << "-"
        << 1900 + now.tm_year;
    timeStream << setw(2) << setfill('0') << now.tm_hour << ":"
        << setw(2) << setfill('0') << now.tm_min;

    return { dateStream.str(), timeStream.str() };
}

void readUsersFromFile(const string& filename) {
    ifstream file(filename);
    if (file.is_open()) {
        string line;
        userCount = 0;
        for (int i = 0; i < 6; i++) {
            getline(file, line);
        }

        while (getline(file, line) && userCount < MAX_USERS && line != "---") {
            istringstream ss(line);
            Client user;
            string isAdminStr;
            ss >> user.userID >> user.firstName >> user.lastName >> user.password
                >> user.email >> user.phone >> isAdminStr;
            user.isAdmin = (isAdminStr == "1");
            user.nbDonations = 0;
            users[userCount++] = user;
        }
    }
}

void saveUsersToFile(const string& filename) {
    ofstream file(filename);
    if (file.is_open()) {
        file << "ID\tFirst name\tLast name\tPassword\tEmail\tPhone number\tIsAdmin\n";
        file << "---\n";

        for (int i = 0; i < userCount; i++) {
            file << users[i].userID << "\t" << users[i].firstName << "\t" << users[i].lastName << "\t"
                << users[i].password << "\t" << users[i].email << "\t" << users[i].phone << "\t"
                << (users[i].isAdmin ? "1" : "0") << "\n";
        }
    }
}

Client* authenticateUser(const string& email, const string& password) {
    string hashedPassword = hashPassword(password);
    for (int i = 0; i < userCount; i++) {
        if (users[i].email == email && users[i].password == hashedPassword) {
            return &users[i];
        }
    }
    return nullptr;
}

void registerUser(bool isAdmin = false) {
    if (userCount >= MAX_USERS) {
        cout << "Maximum number of users reached. Cannot register new user.\n";
        return;
    }

    Client newUser;
    newUser.userID = generateUserID(isAdmin);
    newUser.isAdmin = isAdmin;
    newUser.nbDonations = 0;

    cout << "\n" << (isAdmin ? "Admin " : "") << "Registration:\n";
    cout << "Enter first name: ";
    cin >> newUser.firstName;
    cout << "Enter last name: ";
    cin >> newUser.lastName;

    string password;
    while (true) {
        cout << "Enter password (at least 8 characters): ";
        cin >> password;
        if (password.length() >= 8) {
            newUser.password = hashPassword(password);
            break;
        }
        cout << "Password must be at least 8 characters long.\n";
    }

    while (true) {
        cout << "Enter email: ";
        cin >> newUser.email;
        if (isValidEmail(newUser.email)) {
            bool emailExists = false;
            for (int i = 0; i < userCount; i++) {
                if (users[i].email == newUser.email) {
                    emailExists = true;
                    break;
                }
            }
            if (!emailExists) break;
            cout << "Email already registered.\n";
        }
        else {
            cout << "Invalid email format.\n";
        }
    }

    while (true) {
        cout << "Enter phone number (exactly 8 digits): ";
        cin >> newUser.phone;
        if (isValidPhone(newUser.phone)) break;
        cout << "Invalid phone number format. Must be exactly 8 digits.\n";
    }

    users[userCount++] = newUser;
    saveUsersToFile("users.txt");
    cout << (isAdmin ? "Admin " : "") << "Registration successful!\n";
}

void readCharitiesFromFile(const string& filename) {
    ifstream file(filename);
    if (file.is_open()) {
        string line;
        charityCount = 0;
        for (int i = 0; i < 4; i++) {
            getline(file, line);
        }

        while (getline(file, line) && charityCount < MAX_CHARITIES && line != "---") {
            istringstream ss(line);
            Charity charity;
            ss >> charity.charityID >> charity.name;
            getline(ss, charity.description); 
            charities[charityCount++] = charity;
        }
    }
}

void saveCharitiesToFile(const string& filename) {
    ofstream file(filename);
    if (file.is_open()) {
        file << "Chatty ID\tChatity Name\tStatus\n";
        file << "---\n";

        for (int i = 0; i < charityCount; i++) {
            file << charities[i].charityID << "\t" << charities[i].name << "\t"
                << charities[i].status << "\n";
        }
    }
}

void addCharity(const Charity& charity) {
    if (charityCount >= MAX_CHARITIES) {
        cout << "Maximum number of charities reached.\n";
        return;
    }

    for (int i = 0; i < charityCount; i++) {
        if (charities[i].name == charity.name) {
            cout << "Charity already exists!\n";
            return;
        }
    }
    charities[charityCount++] = charity;
}

void removeCharity(int charityID) {
    for (int i = 0; i < charityCount; i++) {
        if (charities[i].charityID == charityID) {
            for (int j = i; j < charityCount - 1; j++) {
                charities[j] = charities[j + 1];
            }
            charityCount--;
            return;
        }
    }
}

void modifyCharity(int charityID, const Charity& modifiedCharity) {
    for (int i = 0; i < charityCount; i++) {
        if (charities[i].charityID == charityID) {
            charities[i] = modifiedCharity;
            return;
        }
    }
}

void makeDonation(Client& donor, int charityID, double amount, const string& message) {
    if (donor.nbDonations >= MAX_DONATIONS_PER_USER) {
        cout << "Maximum donations reached for this user.\n";
        return;
    }

    for (int i = 0; i < charityCount; i++) {
        if (charities[i].charityID == charityID) {
            charities[i].currentAmount += amount;
            donor.donations[donor.nbDonations++] = {
                donor.nbDonations + 1,
                charityID,
                amount,
                getCurrentDateTime(),
                message
            };
            break;
        }
    }
}

void generateDonationsPDF(const string& filename) {
    HPDF_Doc pdf = HPDF_New(NULL, NULL);
    if (!pdf) {
        cout << "Error creating PDF document\n";
        return;
    }

    try {
        HPDF_Page page = HPDF_AddPage(pdf);
        HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

 
        HPDF_Font font = HPDF_GetFont(pdf, "Helvetica", NULL);
        HPDF_Page_SetFontAndSize(page, font, 12);

        HPDF_Page_BeginText(page);
        HPDF_Page_MoveTextPos(page, 50, HPDF_Page_GetHeight(page) - 50);
        HPDF_Page_ShowText(page, "Donation Report - All Donations");
        HPDF_Page_EndText(page);

        vector<Donation> allDonations;
        for (int i = 0; i < userCount; i++) {
            for (int j = 0; j < users[i].nbDonations; j++) {
                Donation d = users[i].donations[j];
                d.message = users[i].firstName + " " + users[i].lastName; 
                allDonations.push_back(d);
            }
        }

        HPDF_Page_BeginText(page);
        HPDF_Page_MoveTextPos(page, 50, HPDF_Page_GetHeight(page) - 80);

        float y = HPDF_Page_GetHeight(page) - 80;
        for (const auto& d : allDonations) {
            if (y < 100) { 
                HPDF_Page_EndText(page);
                page = HPDF_AddPage(pdf);
                HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
                HPDF_Page_SetFontAndSize(page, font, 12);
                HPDF_Page_BeginText(page);
                HPDF_Page_MoveTextPos(page, 50, HPDF_Page_GetHeight(page) - 50);
                y = HPDF_Page_GetHeight(page) - 50;
            }

            stringstream ss;
            ss << "Donor: " << d.message
                << " | Amount: " << d.amount << "$"
                << " | Date: " << d.d.date << " " << d.d.time
                << " | Charity ID: " << d.charityID;

            HPDF_Page_ShowText(page, ss.str().c_str());
            HPDF_Page_MoveTextPos(page, 0, -20);
            y -= 20;
        }

        HPDF_Page_EndText(page);
        HPDF_SaveToFile(pdf, filename.c_str());
        cout << "PDF report generated: " << filename << endl;
    }
    catch (...) {
        HPDF_Free(pdf);
        throw;
    }

    HPDF_Free(pdf);
}

void saveDonationsToFile(const string& filename) {
    ofstream file(filename);
    if (file.is_open()) {
        file << "DonationID\tCharity ID\tAmount\tDonation Date\tDonation Time\n";
        file << "---\n";

        for (int i = 0; i < userCount; i++) {
            for (int j = 0; j < users[i].nbDonations; j++) {
                Donation& d = users[i].donations[j];
                file << d.donationID << "\t" << d.charityID << "\t" << d.amount << "$\t"
                    << d.d.date << "\t" << d.d.time << "\n";
            }
        }
    }
    string pdfFilename = "donations_report.pdf";
    generateDonationsPDF(pdfFilename);
}

void readDonationsFromFile(const string& filename) {
    ifstream file(filename);
    if (file.is_open()) {
        string line;
        for (int i = 0; i < 4; i++) {
            getline(file, line);
        }

        while (getline(file, line) && line != "---") {
            istringstream ss(line);
            Donation donation;
            char dollarSign;
            ss >> donation.donationID >> donation.charityID >> donation.amount >> dollarSign
                >> donation.d.date >> donation.d.time;

            if (userCount > 0) {
                if (users[0].nbDonations < MAX_DONATIONS_PER_USER) {
                    users[0].donations[users[0].nbDonations++] = donation;
                }
            }
        }
    }
}

void adminMenu() {
    int choice;
    do {
        cout << "\nAdmin Menu:\n";
        cout << "1. Add a charity\n";
        cout << "2. Remove a charity\n";
        cout << "3. Modify a charity\n";
        cout << "4. View all charities\n";
        cout << "5. View all donations\n";
        cout << "6. Logout\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
        case 1: {
            Charity newCharity;
            cout << "Enter charity ID: ";
            cin >> newCharity.charityID;
            cout << "Enter charity name: ";
            cin.ignore();
            getline(cin, newCharity.name);
            cout << "Enter charity description: ";
            getline(cin, newCharity.description);
            newCharity.creationDate = getCurrentDateTime();
            newCharity.status = "Open for donations";
            addCharity(newCharity);
            saveCharitiesToFile("charities.txt");
            break;
        }
        case 2: {
            int charityID;
            cout << "Enter charity ID to remove: ";
            cin >> charityID;
            removeCharity(charityID);
            saveCharitiesToFile("charities.txt");
            break;
        }
        case 3: {
            int charityID;
            cout << "Enter charity ID to modify: ";
            cin >> charityID;
            Charity modifiedCharity;
            cout << "Enter new charity ID: ";
            cin >> modifiedCharity.charityID;
            cout << "Enter new charity name: ";
            cin.ignore();
            getline(cin, modifiedCharity.name);
            cout << "Enter new description: ";
            getline(cin, modifiedCharity.description);
            modifiedCharity.creationDate = getCurrentDateTime();
            modifiedCharity.status = "Open for donations";
            modifyCharity(charityID, modifiedCharity);
            saveCharitiesToFile("charities.txt");
            break;
        }
        case 4:
            for (int i = 0; i < charityCount; i++) {
                cout << "ID: " << charities[i].charityID << ", Name: " << charities[i].name
                    << ", Description: " << charities[i].description
                    << ", Status: " << charities[i].status << "\n";
            }
            break;
        case 5: {
            cout << "All Donations:\n";
            for (int i = 0; i < userCount; i++) {
                for (int j = 0; j < users[i].nbDonations; j++) {
                    Donation& d = users[i].donations[j];
                    cout << "User: " << users[i].firstName << " " << users[i].lastName
                        << " | Donation ID: " << d.donationID << ", Charity: " << d.charityID
                        << ", Amount: " << d.amount << "$, Date: " << d.d.date
                        << " " << d.d.time << "\n";
                }
            }
            break;
        }
        case 6:
            break;
        default:
            cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 6);
}

void donorMenu(Client& currentUser) {
    int choice;
    do {
        cout << "\nDonor Menu:\n";
        cout << "1. Browse charities\n";
        cout << "2. Make a donation\n";
        cout << "3. View my donations\n";
        cout << "4. Modify donation\n";
        cout << "5. Cancel donation\n";
        cout << "6. Logout\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) { 
        case 1:
            for (int i = 0; i < charityCount; i++) {
                cout << "ID: " << charities[i].charityID << ", Name: " << charities[i].name
                    << ", Description: " << charities[i].description
                    << ", Status: " << charities[i].status << "\n";
            }
            break;
        case 2: {
            int charityID;
            double amount;
            string message;
            cout << "Enter charity ID: ";
            cin >> charityID;
            cout << "Enter donation amount: ";
            cin >> amount;
            cout << "Enter an optional message for your donation: ";
            cin.ignore();
            getline(cin, message);
            makeDonation(currentUser, charityID, amount, message);
            saveDonationsToFile("donations.txt");
            break;
        }
        case 3: {
            cout << "Your donations:\n";
            for (int i = 0; i < currentUser.nbDonations; i++) {
                Donation& d = currentUser.donations[i];
                cout << "ID: " << d.donationID << ", Charity: " << d.charityID
                    << ", Amount: " << d.amount << "$, Date: " << d.d.date
                    << " " << d.d.time << ", Message: " << d.message << "\n";
            }
            break;
        }
        case 4: {
            int donationID;
            cout << "Enter donation ID to modify: ";
            cin >> donationID;
            double newAmount;
            int newCharityID;
            string newMessage;
            cout << "Enter new donation amount: ";
            cin >> newAmount;
            cout << "Enter new charity ID: ";
            cin >> newCharityID;
            cout << "Enter new message: ";
            cin.ignore();
            getline(cin, newMessage);
            bool found = false;
            for (int i = 0; i < currentUser.nbDonations; i++) {
                if (currentUser.donations[i].donationID == donationID) {
                    currentUser.donations[i].amount = newAmount;
                    currentUser.donations[i].charityID = newCharityID;
                    currentUser.donations[i].d = getCurrentDateTime();
                    currentUser.donations[i].message = newMessage;
                    found = true;
                    break;
                }
            }
            if (found) {
                saveDonationsToFile("donations.txt");
                cout << "Donation modified.\n";
            }
            else {
                cout << "Donation not found.\n";
            }
            break;
        }
        case 5: {
            int donationID;
            cout << "Enter donation ID to cancel: ";
            cin >> donationID;

            bool found = false;
            for (int i = 0; i < currentUser.nbDonations; i++) {
                if (currentUser.donations[i].donationID == donationID) {
                    for (int j = i; j < currentUser.nbDonations - 1; j++) {
                        currentUser.donations[j] = currentUser.donations[j + 1];
                    }
                    currentUser.nbDonations--;
                    found = true;
                    break;
                }
            }

            if (found) {
                saveDonationsToFile("donations.txt");
                cout << "Donation canceled successfully.\n";
            }
            else {
                cout << "Donation not found.\n";
            }
            break;
        }
        case 6:
            break;
        default:
            cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 6);
}

void run() {
    readCharitiesFromFile("charities.txt");
    readUsersFromFile("users.txt");
    readDonationsFromFile("donations.txt");

    if (charityCount == 0) {
        charities[0].charityID = 101;
        charities[0].name = "Unity Uplift";
        charities[0].description = "Committed to fostering positive social change";
        charities[0].status = "Open for donations";
        charityCount = 1;
        saveCharitiesToFile("charities.txt");
    }

    int choice;
    do {
        cout << "\nMain Menu:\n";
        cout << "1. Login\n";
        cout << "2. Admin Login\n";
        cout << "3. Register\n";
        cout << "4. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
        case 1: {
            string email, password;
            cout << "Enter your email: ";
            cin >> email;
            cout << "Enter your password: ";
            cin >> password;

            Client* authenticatedUser = authenticateUser(email, password);
            if (authenticatedUser != nullptr) {
                cout << "Login successful\n";
                donorMenu(*authenticatedUser);
            }
            else {
                cout << "Invalid email or password\n";
            }
            break;
        }
        case 2: {
            string email, password;
            cout << "Enter your admin email: ";
            cin >> email;
            cout << "Enter your admin password: ";
            cin >> password;

            Client* authenticatedAdmin = authenticateUser(email, password);
            if (authenticatedAdmin != nullptr && authenticatedAdmin->isAdmin) {
                cout << "Admin login successful\n";
                adminMenu();
            }
            else {
                cout << "Invalid admin credentials or insufficient privileges\n";
            }
            break;
        }
        case 3: {
            int regChoice;
            cout << "\nRegistration Type:\n";
            cout << "1. Donor Registration\n";
            cout << "2. Admin Registration\n";
            cout << "Enter your choice: ";
            cin >> regChoice;

            if (regChoice == 1) {
                registerUser(false); 
            }
            else if (regChoice == 2) {

                string adminCode;
                cout << "Enter admin registration code: ";
                cin >> adminCode;

                if (adminCode == "ADMIN123") {
                    registerUser(true); 
                }
                else {
                    cout << "Invalid admin registration code.\n";
                }
            }
            else {
                cout << "Invalid choice.\n";
            }
            break;
        }
        case 4:
            break;
        default:
            cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 4);
}

int main() {
    run();
    return 0;
}