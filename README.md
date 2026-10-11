# Charity Donation Management System

A professional C++ application designed to manage charitable donations, user accounts, and administrative oversight. This system provides a secure way for donors to contribute to various causes and for administrators to manage the available charities.

## Features

###  User & Donor Management
- **Secure Authentication**: User passwords are encrypted using **SHA-256 hashing** (via OpenSSL) to ensure data privacy.
- **User Registration**: Support for both standard Donor and privileged Admin accounts.
- **Input Validation**: Integrated regular expression (regex) validation for email formats and phone numbers.
- **Personal Dashboard**: Donors can view their full donation history, modify existing donations, or cancel them.

###  Charity Management
- **Charity Directory**: A browseable list of charities including descriptions and current statuses.
- **Administrative Control**: Admins can add new charities, remove existing ones, or modify charity details.
- **Dynamic Tracking**: Real-time tracking of funds raised for each charity.

###  Donation System
- **Seamless Contributions**: Donors can select a charity, specify an amount, and leave an optional personal message.
- **Automated Record Keeping**: All transactions are timestamped with date and time.
- **Data Persistence**: User, charity, and donation data are persisted across sessions using text-based file storage.

### Reporting & Documentation
- **PDF Report Generation**: Automatically generates a professional `donations_report.pdf` containing a comprehensive list of all donations using the `libharu` library.
- **File-based Backups**: Maintains structured `.txt` files for all critical system data.

## 🛠️ Technical Stack
- **Language**: C++
- **Security**: OpenSSL (SHA-256)
- **PDF Generation**: libharu (hpdf)
- **Operating System**: Windows (Developed with Visual Studio)

##  Prerequisites
To compile and run this project, you will need:
- A C++ compiler (e.g., MSVC, GCC, or Clang)
- **OpenSSL Library**: For password hashing.
- **libharu Library**: For PDF generation.

## Installation & Setup
1. Clone the repository:
   ```bash
   git clone <repository-url>
   ```
2. Configure your build environment to link against `libssl`, `libcrypto`, and `libhpdf`.
3. Build the project using your preferred IDE (Visual Studio `.sln` provided) or command line.
4. Run the executable `Project3.exe`.

## Admin Access
To register as an administrator during the registration process, use the following secret code:
`ADMIN123`
