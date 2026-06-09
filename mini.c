#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define ROWS 25
#define COLS 60
#define MAX_OBJECTS 100
// ANSI Color Escape Sequences for a beautiful terminal UI
#define ANSI_COLOR_RESET   "\033[0m"
#define ANSI_COLOR_CYAN    "\033[1;36m"
#define ANSI_COLOR_GREEN   "\033[1;32m"
#define ANSI_COLOR_YELLOW  "\033[1;33m"
#define ANSI_COLOR_RED     "\033[1;31m"
#define ANSI_COLOR_BLUE    "\033[1;34m"
#define ANSI_COLOR_MAGENTA "\033[1;35m"
#define ANSI_COLOR_GRAY    "\033[38;5;242m"
#define ANSI_CLEAR_SCREEN  "\033[H\033[J"
// Shape types enum
typedef enum {
    SHAPE_LINE = 1,
    SHAPE_RECTANGLE,
    SHAPE_CIRCLE,
    SHAPE_TRIANGLE
} ShapeType;
// Coordinate parameters for each shape type
typedef struct {
    int x1, y1;
    int x2, y2;
} LineParams;
typedef struct {
    int x1, y1;
    int x2, y2;
} RectParams;
typedef struct {
    int cx, cy;
    int r;
} CircleParams;
typedef struct {
    int x1, y1;
    int x2, y2;
    int x3, y3;
} TriangleParams;
// Graphic Object structure
typedef struct {
    int id;
    int active;
    ShapeType type;
    union {
        LineParams line;
        RectParams rect;
        CircleParams circle;
        TriangleParams triangle;
    } params;
} ShapeObject;
// Global variables
char canvas[ROWS][COLS];
ShapeObject objects[MAX_OBJECTS];
int next_object_id = 1;
// Function declarations
void enable_ansi_escapes();
void clear_canvas();
void draw_pixel(int x, int y);
void draw_line(int x1, int y1, int x2, int y2);
void draw_circle(int cx, int cy, int r);
void draw_rectangle(int x1, int y1, int x2, int y2);
void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3);
void render_all_objects();
void display_picture();
void list_active_objects();
int read_int(const char *prompt, int min_val, int max_val);
void add_object_menu();
void delete_object_menu();
void modify_object_menu();
void show_help();
int main() {
    enable_ansi_escapes();
    
    // Initialize objects list
    for (int i = 0; i < MAX_OBJECTS; i++) {
        objects[i].active = 0;
    }
    
    // Render empty canvas initially
    render_all_objects();
    
    int choice = 0;
    while (choice != 6) {
        printf(ANSI_CLEAR_SCREEN);
        printf(ANSI_COLOR_CYAN "==============================================================\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_CYAN "                  ★ 2D VECTOR GRAPHICS EDITOR ★              \n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_CYAN "==============================================================\n" ANSI_COLOR_RESET);
        
        display_picture();
        list_active_objects();
        
        printf("\n" ANSI_COLOR_YELLOW "Menu Options:\n" ANSI_COLOR_RESET);
        printf("  1. Add an object\n");
        printf("  2. Delete an object\n");
        printf("  3. Modify an object\n");
        printf("  4. Clear canvas (Delete all objects)\n");
        printf("  5. Help / Instructions\n");
        printf("  6. Exit Program\n");
        printf("--------------------------------------------------------------\n");
        
        choice = read_int("Enter your choice (1-6): ", 1, 6);
        
        switch (choice) {
            case 1:
                add_object_menu();
                break;
            case 2:
                delete_object_menu();
                break;
            case 3:
                modify_object_menu();
                break;
            case 4: {
                int confirm = read_int("Are you sure you want to delete all objects? (1 for Yes, 0 for No): ", 0, 1);
                if (confirm) {
                    for (int i = 0; i < MAX_OBJECTS; i++) {
                        objects[i].active = 0;
                    }
                    next_object_id = 1;
                    render_all_objects();
                    printf(ANSI_COLOR_GREEN "Canvas cleared successfully!\n" ANSI_COLOR_RESET);
                }
                break;
            }
            case 5:
                show_help();
                break;
            case 6:
                printf("\nThank you for using the 2D Graphics Editor! Goodbye.\n");
                break;
        }
    }
    
    return 0;
}
// Enable ANSI escape processing in Windows console if applicable
void enable_ansi_escapes() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}
// Fills the 2D character canvas with underscores
void clear_canvas() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            canvas[i][j] = '_';
        }
    }
}
// Safely sets a character in the canvas if it falls within boundaries
void draw_pixel(int x, int y) {
    if (x >= 0 && x < COLS && y >= 0 && y < ROWS) {
        canvas[y][x] = '*';
    }
}
// Bresenham's Line Algorithm
void draw_line(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    while (1) {
        draw_pixel(x1, y1);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}
// Helper octant drawer for circle drawing
static void draw_circle_points(int cx, int cy, int x, int y) {
    draw_pixel(cx + x, cy + y);
    draw_pixel(cx - x, cy + y);
    draw_pixel(cx + x, cy - y);
    draw_pixel(cx - x, cy - y);
    draw_pixel(cx + y, cy + x);
    draw_pixel(cx - y, cy + x);
    draw_pixel(cx + y, cy - x);
    draw_pixel(cx - y, cy - x);
}
// Bresenham's Circle Algorithm (Midpoint Circle Algorithm)
void draw_circle(int cx, int cy, int r) {
    if (r < 0) return;
    if (r == 0) {
        draw_pixel(cx, cy);
        return;
    }
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;
    draw_circle_points(cx, cy, x, y);
    while (y >= x) {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
        draw_circle_points(cx, cy, x, y);
    }
}
// Draw rectangle borders
void draw_rectangle(int x1, int y1, int x2, int y2) {
    draw_line(x1, y1, x2, y1); // Top
    draw_line(x1, y2, x2, y2); // Bottom
    draw_line(x1, y1, x1, y2); // Left
    draw_line(x2, y1, x2, y2); // Right
}
// Draw triangle from three points
void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3) {
    draw_line(x1, y1, x2, y2);
    draw_line(x2, y2, x3, y3);
    draw_line(x3, y3, x1, y1);
}
// Re-rasterize all active shape objects onto the 2D canvas
void render_all_objects() {
    clear_canvas();
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i].active) {
            switch (objects[i].type) {
                case SHAPE_LINE:
                    draw_line(objects[i].params.line.x1, objects[i].params.line.y1,
                              objects[i].params.line.x2, objects[i].params.line.y2);
                    break;
                case SHAPE_RECTANGLE:
                    draw_rectangle(objects[i].params.rect.x1, objects[i].params.rect.y1,
                                   objects[i].params.rect.x2, objects[i].params.rect.y2);
                    break;
                case SHAPE_CIRCLE:
                    draw_circle(objects[i].params.circle.cx, objects[i].params.circle.cy,
                                objects[i].params.circle.r);
                    break;
                case SHAPE_TRIANGLE:
                    draw_triangle(objects[i].params.triangle.x1, objects[i].params.triangle.y1,
                                  objects[i].params.triangle.x2, objects[i].params.triangle.y2,
                                  objects[i].params.triangle.x3, objects[i].params.triangle.y3);
                    break;
            }
        }
    }
}
// Display the 2D character canvas with borders and coordinate guides
void display_picture() {
    // Print column guide markers in groups of 5 columns
    printf("    ");
    for (int j = 0; j < COLS; j++) {
        if (j % 5 == 0) {
            printf("%-5d", j);
        }
    }
    printf("\n");
    
    // Print upper boundary frame
    printf("   ┌");
    for (int j = 0; j < COLS; j++) {
        printf("─");
    }
    printf("┐\n");
    
    // Print rows with sidebar row guides and character pixels
    for (int i = 0; i < ROWS; i++) {
        printf("%2d │", i);
        for (int j = 0; j < COLS; j++) {
            if (canvas[i][j] == '*') {
                printf(ANSI_COLOR_GREEN "*" ANSI_COLOR_RESET); // Highlight graphical objects in bold green
            } else {
                printf(ANSI_COLOR_GRAY "_" ANSI_COLOR_RESET);  // Keep background dim gray
            }
        }
        printf("│\n");
    }
    
    // Print bottom boundary frame
    printf("   └");
    for (int j = 0; j < COLS; j++) {
        printf("─");
    }
    printf("┘\n");
}
// Prints the catalog of active objects and their parameters
void list_active_objects() {
    int count = 0;
    printf("\n" ANSI_COLOR_YELLOW "Active Objects:\n" ANSI_COLOR_RESET);
    printf("--------------------------------------------------------------\n");
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i].active) {
            count++;
            printf("  ID " ANSI_COLOR_CYAN "[%d]" ANSI_COLOR_RESET " ", objects[i].id);
            switch (objects[i].type) {
                case SHAPE_LINE:
                    printf("Line: from (%d, %d) to (%d, %d)\n",
                           objects[i].params.line.x1, objects[i].params.line.y1,
                           objects[i].params.line.x2, objects[i].params.line.y2);
                    break;
                case SHAPE_RECTANGLE:
                    printf("Rectangle: Corner 1 (%d, %d) to Corner 2 (%d, %d)\n",
                           objects[i].params.rect.x1, objects[i].params.rect.y1,
                           objects[i].params.rect.x2, objects[i].params.rect.y2);
                    break;
                case SHAPE_CIRCLE:
                    printf("Circle: Center (%d, %d), Radius %d\n",
                           objects[i].params.circle.cx, objects[i].params.circle.cy,
                           objects[i].params.circle.r);
                    break;
                case SHAPE_TRIANGLE:
                    printf("Triangle: Vertices (%d, %d), (%d, %d), (%d, %d)\n",
                           objects[i].params.triangle.x1, objects[i].params.triangle.y1,
                           objects[i].params.triangle.x2, objects[i].params.triangle.y2,
                           objects[i].params.triangle.x3, objects[i].params.triangle.y3);
                    break;
            }
        }
    }
    if (count == 0) {
        printf("  (No objects added yet. Canvas is empty!)\n");
    }
    printf("--------------------------------------------------------------\n");
}
// Robust input helper: prevents scan errors, handles empty strings, validates range
int read_int(const char *prompt, int min_val, int max_val) {
    int value;
    char buffer[128];
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            printf(ANSI_COLOR_RED "Error reading input. Please try again.\n" ANSI_COLOR_RESET);
            continue;
        }
        
        // Trim trailing newlines and carriage returns
        buffer[strcspn(buffer, "\r\n")] = '\0';
        
        if (strlen(buffer) == 0) {
            printf(ANSI_COLOR_RED "Input cannot be empty. Please enter a number.\n" ANSI_COLOR_RESET);
            continue;
        }
        
        char *endptr;
        long val = strtol(buffer, &endptr, 10);
        
        // If endptr doesn't point to string termination, input had non-numeric elements
        if (*endptr != '\0') {
            printf(ANSI_COLOR_RED "Invalid character found. Please enter a whole number.\n" ANSI_COLOR_RESET);
            continue;
        }
        
        if (val < min_val || val > max_val) {
            printf(ANSI_COLOR_RED "Out of range! Please enter a value from %d to %d.\n" ANSI_COLOR_RESET, min_val, max_val);
            continue;
        }
        
        value = (int)val;
        break;
    }
    return value;
}
// Add Object Submenu
void add_object_menu() {
    printf("\n" ANSI_COLOR_YELLOW "Add Object Mode:\n" ANSI_COLOR_RESET);
    printf("  1. Line\n");
    printf("  2. Rectangle\n");
    printf("  3. Circle\n");
    printf("  4. Triangle\n");
    printf("  5. Back to Main Menu\n");
    
    int type_choice = read_int("Select shape type (1-5): ", 1, 5);
    if (type_choice == 5) return;
    
    // Find empty slot in our active list
    int slot = -1;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (!objects[i].active) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        printf(ANSI_COLOR_RED "Error: Canvas memory full! (Max %d objects reached). Delete objects first.\n" ANSI_COLOR_RESET, MAX_OBJECTS);
        printf("Press Enter to continue...");
        getchar();
        return;
    }
    
    ShapeObject obj;
    obj.id = next_object_id++;
    obj.active = 1;
    obj.type = (ShapeType)type_choice;
    
    printf("\nEnter parameters (Canvas is %dx%d):\n", COLS, ROWS);
    printf("X-coordinate: 0 to %d (horizontal)\n", COLS - 1);
    printf("Y-coordinate: 0 to %d (vertical)\n\n", ROWS - 1);
    
    switch (obj.type) {
        case SHAPE_LINE:
            obj.params.line.x1 = read_int("Enter X1: ", 0, COLS - 1);
            obj.params.line.y1 = read_int("Enter Y1: ", 0, ROWS - 1);
            obj.params.line.x2 = read_int("Enter X2: ", 0, COLS - 1);
            obj.params.line.y2 = read_int("Enter Y2: ", 0, ROWS - 1);
            break;
            
        case SHAPE_RECTANGLE:
            obj.params.rect.x1 = read_int("Enter Corner 1 X1: ", 0, COLS - 1);
            obj.params.rect.y1 = read_int("Enter Corner 1 Y1: ", 0, ROWS - 1);
            obj.params.rect.x2 = read_int("Enter Corner 2 X2: ", 0, COLS - 1);
            obj.params.rect.y2 = read_int("Enter Corner 2 Y2: ", 0, ROWS - 1);
            break;
            
        case SHAPE_CIRCLE:
            obj.params.circle.cx = read_int("Enter Center X: ", 0, COLS - 1);
            obj.params.circle.cy = read_int("Enter Center Y: ", 0, ROWS - 1);
            // Limit radius to twice the canvas dimension to prevent absurd values
            obj.params.circle.r = read_int("Enter Radius: ", 0, COLS);
            break;
            
        case SHAPE_TRIANGLE:
            obj.params.triangle.x1 = read_int("Enter Vertex 1 X1: ", 0, COLS - 1);
            obj.params.triangle.y1 = read_int("Enter Vertex 1 Y1: ", 0, ROWS - 1);
            obj.params.triangle.x2 = read_int("Enter Vertex 2 X2: ", 0, COLS - 1);
            obj.params.triangle.y2 = read_int("Enter Vertex 2 Y2: ", 0, ROWS - 1);
            obj.params.triangle.x3 = read_int("Enter Vertex 3 X3: ", 0, COLS - 1);
            obj.params.triangle.y3 = read_int("Enter Vertex 3 Y3: ", 0, ROWS - 1);
            break;
    }
    
    objects[slot] = obj;
    render_all_objects();
    
    printf(ANSI_COLOR_GREEN "\nShape added successfully with ID [%d]!\n" ANSI_COLOR_RESET, obj.id);
    printf("Press Enter to continue...");
    getchar();
}
// Delete Object Submenu
void delete_object_menu() {
    int id_to_delete = read_int("Enter the ID of the object to delete (0 to cancel): ", 0, next_object_id);
    if (id_to_delete == 0) return;
    
    int found = 0;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i].active && objects[i].id == id_to_delete) {
            objects[i].active = 0;
            found = 1;
            break;
        }
    }
    
    if (found) {
        render_all_objects();
        printf(ANSI_COLOR_GREEN "Object ID [%d] deleted successfully!\n" ANSI_COLOR_RESET, id_to_delete);
    } else {
        printf(ANSI_COLOR_RED "Object with ID [%d] not found.\n" ANSI_COLOR_RESET, id_to_delete);
    }
    printf("Press Enter to continue...");
    getchar();
}
// Modify Object Submenu
void modify_object_menu() {
    int id_to_modify = read_int("Enter the ID of the object to modify (0 to cancel): ", 0, next_object_id);
    if (id_to_modify == 0) return;
    
    int slot = -1;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i].active && objects[i].id == id_to_modify) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        printf(ANSI_COLOR_RED "Object with ID [%d] not found.\n" ANSI_COLOR_RESET, id_to_modify);
        printf("Press Enter to continue...");
        getchar();
        return;
    }
    
    printf("\nModifying Object ID [%d]:\n", id_to_modify);
    printf("Enter new parameters (Canvas is %dx%d):\n", COLS, ROWS);
    
    switch (objects[slot].type) {
        case SHAPE_LINE:
            printf("Current Line: (%d, %d) to (%d, %d)\n",
                   objects[slot].params.line.x1, objects[slot].params.line.y1,
                   objects[slot].params.line.x2, objects[slot].params.line.y2);
            objects[slot].params.line.x1 = read_int("Enter new X1: ", 0, COLS - 1);
            objects[slot].params.line.y1 = read_int("Enter new Y1: ", 0, ROWS - 1);
            objects[slot].params.line.x2 = read_int("Enter new X2: ", 0, COLS - 1);
            objects[slot].params.line.y2 = read_int("Enter new Y2: ", 0, ROWS - 1);
            break;
            
        case SHAPE_RECTANGLE:
            printf("Current Rectangle: (%d, %d) to (%d, %d)\n",
                   objects[slot].params.rect.x1, objects[slot].params.rect.y1,
                   objects[slot].params.rect.x2, objects[slot].params.rect.y2);
            objects[slot].params.rect.x1 = read_int("Enter new Corner 1 X1: ", 0, COLS - 1);
            objects[slot].params.rect.y1 = read_int("Enter new Corner 1 Y1: ", 0, ROWS - 1);
            objects[slot].params.rect.x2 = read_int("Enter new Corner 2 X2: ", 0, COLS - 1);
            objects[slot].params.rect.y2 = read_int("Enter new Corner 2 Y2: ", 0, ROWS - 1);
            break;
            
        case SHAPE_CIRCLE:
            printf("Current Circle: Center (%d, %d), Radius %d\n",
                   objects[slot].params.circle.cx, objects[slot].params.circle.cy,
                   objects[slot].params.circle.r);
            objects[slot].params.circle.cx = read_int("Enter new Center X: ", 0, COLS - 1);
            objects[slot].params.circle.cy = read_int("Enter new Center Y: ", 0, ROWS - 1);
            objects[slot].params.circle.r = read_int("Enter new Radius: ", 0, COLS);
            break;
            
        case SHAPE_TRIANGLE:
            printf("Current Triangle: (%d, %d), (%d, %d), (%d, %d)\n",
                   objects[slot].params.triangle.x1, objects[slot].params.triangle.y1,
                   objects[slot].params.triangle.x2, objects[slot].params.triangle.y2,
                   objects[slot].params.triangle.x3, objects[slot].params.triangle.y3);
            objects[slot].params.triangle.x1 = read_int("Enter new Vertex 1 X1: ", 0, COLS - 1);
            objects[slot].params.triangle.y1 = read_int("Enter new Vertex 1 Y1: ", 0, ROWS - 1);
            objects[slot].params.triangle.x2 = read_int("Enter new Vertex 2 X2: ", 0, COLS - 1);
            objects[slot].params.triangle.y2 = read_int("Enter new Vertex 2 Y2: ", 0, ROWS - 1);
            objects[slot].params.triangle.x3 = read_int("Enter new Vertex 3 X3: ", 0, COLS - 1);
            objects[slot].params.triangle.y3 = read_int("Enter new Vertex 3 Y3: ", 0, ROWS - 1);
            break;
    }
    
    render_all_objects();
    printf(ANSI_COLOR_GREEN "Object ID [%d] modified successfully!\n" ANSI_COLOR_RESET, id_to_modify);
    printf("Press Enter to continue...");
    getchar();
}
// Show Help / Instructions Dialog
void show_help() {
    printf(ANSI_CLEAR_SCREEN);
    printf(ANSI_COLOR_YELLOW "==============================================================\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_YELLOW "                  2D GRAPHICS EDITOR GUIDE                    \n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_YELLOW "==============================================================\n" ANSI_COLOR_RESET);
    printf("\n");
    printf(" This program allows you to draw 2D vector shapes onto a \n");
    printf(" grid canvas. All coordinates are whole numbers.\n\n");
    printf("  " ANSI_COLOR_GREEN "Grid Coordinates:" ANSI_COLOR_RESET "\n");
    printf("    • Width (Columns): 0 to %d (left-to-right)\n", COLS - 1);
    printf("    • Height (Rows):   0 to %d (top-to-bottom)\n\n", ROWS - 1);
    printf("  " ANSI_COLOR_GREEN "Shapes and Parameters:" ANSI_COLOR_RESET "\n");
    printf("    • Line: Requires start point (X1, Y1) and end point (X2, Y2).\n");
    printf("    • Rectangle: Defined by two diagonal opposite corners.\n");
    printf("    • Circle: Requires a center point (cx, cy) and radius (r).\n");
    printf("    • Triangle: Requires three vertices.\n\n");
    printf("  " ANSI_COLOR_GREEN "Adding/Deleting/Modifying:" ANSI_COLOR_RESET "\n");
    printf("    • Each shape you create gets a unique ID number.\n");
    printf("    • To edit or remove a shape, select option 2 or 3 and \n");
    printf("      enter the corresponding ID of the shape.\n");
    printf("    • The editor dynamically redraws the entire canvas so \n");
    printf("      that edits are perfectly reflected without artifacts.\n\n");
    printf("--------------------------------------------------------------\n");
    printf("Press Enter to return to the Main Menu...");
    getchar();
}
