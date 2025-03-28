# UI Design Plan for Kiosk Screens

This document outlines the planned redesign for each kiosk display screen using advanced LVGL capabilities to create more aesthetic and user-friendly interfaces.

## General Design Guidelines

- Use a modern, clean aesthetic with rounded corners and soft shadows
- Implement a consistent color scheme across all screens
- Use animations for screen transitions and state changes
- Include visual feedback for user actions
- Design for the 240x240 circular GC9A01 display

## Color Palette

- Primary: #3498db (Blue) - Used for primary actions and main UI elements
- Success: #2ecc71 (Green) - Used for success states and confirmations
- Warning: #f39c12 (Yellow/Orange) - Used for warnings and validation states
- Error: #e74c3c (Red) - Used for error states and important alerts
- Neutral: #ecf0f1 (Light Gray) - Used for backgrounds and non-interactive elements
- Text: #2c3e50 (Dark Blue/Gray) - Used for text elements

## Required Assets

We will need the following assets in PNG format, properly sized for the 240x240 display:

1. `id_card.png` - ID card icon for scanning screens
2. `check.png` - Checkmark icon for success screens
3. `error.png` - Error icon for failure states
4. `arrow.png` - Arrow icon for navigation and flow indication
5. `admin.png` - Admin icon for admin-related screens
6. `database.png` - Database icon for validation screens
7. `keypad.png` - Keypad icon for input screens
8. `card_write.png` - Card writing icon for admin card operations
9. `wifi.png` - WiFi icon for connection status

All icons should be 64x64 pixels with transparent backgrounds, in a flat, modern style.

## Screen Designs

### 1. Idle Screen

A minimal, attractive standby screen that invites users to approach.

**Design Elements:**
- Dark blue gradient background with subtle radial effect
- Pulsing "Approach to Begin" text in the center
- Small school/institution logo at the bottom
- Low-opacity geometric patterns for visual interest

### 2. User Detected Screen

A clear instruction screen for the user to scan their ID card.

**Design Elements:**
- Light blue background
- Large ID card icon in the center (animated with slight floating motion)
- "Scan Your ID Card" text below the icon
- Progress indicator at the bottom showing the system is ready
- Card scanning zone indicator with subtle glow effect

### 3. Database Validation Screen

An animated loading screen while checking the database.

**Design Elements:**
- Yellow/orange background
- Centered spinning circular progress indicator
- "Validating..." text
- Database icon with animated dots moving between user and database icons
- Loading progress bar (indeterminate)

### 4. Check-In Success Screen

A celebratory confirmation screen for successful check-ins.

**Design Elements:**
- Green background
- Large animated checkmark icon (scales in with bounce effect)
- "Check-In Successful" text
- User's name displayed when available
- Current time/date information
- Countdown indicator showing how long the confirmation will display

### 5. Check-Out Success Screen

Similar to check-in but with different messaging.

**Design Elements:**
- Green background
- Large animated checkmark icon
- "Check-Out Successful" text
- User's name displayed when available
- Session duration information (if available)
- Countdown indicator before returning to idle

### 6. Validation Failure Screen

Clear error feedback for failed validation attempts.

**Design Elements:**
- Red background
- Animated error icon (exclamation or X symbol)
- "Validation Failed" text
- Specific error message when available
- "Please Try Again" instruction
- Button or instruction for getting help

### 7. Error Screen

General error indication for system issues.

**Design Elements:**
- Red background
- Warning/error icon
- "System Error" text
- Error code or description when available
- "Please Contact Administrator" instruction
- Admin contact information

## Admin Mode Screens

### 8. Admin Enter ID Screen

Screen for entering an ID to program a new card.

**Design Elements:**
- Blue background
- Keypad visualization graphic
- "Enter Student ID" text
- Input field showing entered digits
- Backspace and Cancel buttons
- Progress/step indicator (Step 1 of 3)

### 9. Admin ID Validating Screen

Loading screen while validating the entered ID.

**Design Elements:**
- Yellow background
- Spinning progress indicator
- "Validating ID..." text
- Database icon with animated connection
- Cancel button

### 10. Admin Tap Card Screen

Screen instructing to place a blank card for programming.

**Design Elements:**
- Blue background
- Animation showing a hand tapping a card to the reader
- "Place Blank Card on Reader" text
- Progress/step indicator (Step 2 of 3)
- Cancel button

### 11. Card Write Success Screen

Confirmation of successful card programming.

**Design Elements:**
- Green background
- Animated checkmark with card icon
- "Card Successfully Programmed" text
- User information summary (ID, name if available)
- "Remove Card" instruction
- Progress/step indicator (Step 3 of 3 - Complete)

### 12. ID Enter Error Screen

Error screen for ID input issues.

**Design Elements:**
- Red background
- Error icon
- "ID Error" text
- Specific error message (e.g., "ID not found", "Invalid format")
- "Try Again" button
- "Cancel" button

### 13. Card Write Error Screen

Error screen for card writing failures.

**Design Elements:**
- Red background
- Error icon with card graphic
- "Card Write Failed" text
- Specific error message when available
- "Try Again" button
- "Cancel" button

### 14. Admin Error Screen

General admin mode error screen.

**Design Elements:**
- Red background
- Admin icon with error symbol
- "Admin Mode Error" text
- Error description when available
- "Exit Admin Mode" button
- Error code for support

## Implementation Approach

For each screen, we will:

1. Create a base container with the appropriate background
2. Add necessary UI elements (icons, text, animations)
3. Implement any required interactivity
4. Define animations for transitions and state changes
5. Ensure consistent styling across all screens

We will use LVGL's object-oriented approach to create reusable components and styles that can be applied across multiple screens to maintain consistency and reduce code duplication. 