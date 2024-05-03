#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <ctype.h>

#define MAX_FILENAME_LENGTH 100
#define PRODUCTS_DIRECTORY "products/"
#define CART_FILENAME "cart.txt"
#define RECEIPT_FILENAME "receipt.txt"


int is_number(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) {
            return 0;
        }
    }
    return 1;
}

void seller_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *window = data;
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *grid;
    GtkWidget *name_label, *price_label, *origin_label;
    GtkWidget *name_entry, *price_entry, *origin_entry;
    GtkWidget *button;
    gchar *product_name, *price_str, *origin_str;
    FILE *fp;
    char filename[MAX_FILENAME_LENGTH];

    dialog = gtk_dialog_new_with_buttons("Product Information", NULL, GTK_DIALOG_MODAL, "Save", GTK_RESPONSE_ACCEPT, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 300);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    grid = gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    name_label = gtk_label_new("Nom du produit :");
    name_entry = gtk_entry_new();
    price_label = gtk_label_new("Prix du produit :");
    price_entry = gtk_entry_new();
    origin_label = gtk_label_new("Provenance du produit :");
    origin_entry = gtk_entry_new();

    gtk_container_add(GTK_CONTAINER(content_area), name_label);
    gtk_container_add(GTK_CONTAINER(content_area), name_entry);
    gtk_container_add(GTK_CONTAINER(content_area), price_label);
    gtk_container_add(GTK_CONTAINER(content_area), price_entry);
    gtk_container_add(GTK_CONTAINER(content_area),origin_label);
    gtk_container_add(GTK_CONTAINER(content_area),origin_entry);
    gtk_widget_show_all(dialog);

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));

    if (result == GTK_RESPONSE_ACCEPT) {
        product_name = gtk_entry_get_text(GTK_ENTRY(name_entry));
        price_str = gtk_entry_get_text(GTK_ENTRY(price_entry));
        origin_str = gtk_entry_get_text(GTK_ENTRY(origin_entry));

        if (strlen(product_name) == 0 || strlen(origin_str) == 0 || strlen(price_str) == 0) {
            printf("Product's info cannot be empty.\n");
            gtk_widget_destroy(dialog);
            return;
        }

        if (!is_number(price_str)) {
            printf("Price must be a valid number.\n");
            gtk_widget_destroy(dialog);
            return;
        }
        sprintf(filename,"./products/%s.txt", product_name);

        fp = fopen(filename, "w");
        if (fp == NULL) {
            printf("Failed to open file.\n");
            return;
        }
        fprintf(fp, "Nom du produit : %s\nPrix du produit : %s\nProvenance du produit : %s", product_name, price_str,origin_str);
        fclose(fp);

        printf("Product information saved successfully.\n");
    }

    gtk_widget_destroy(dialog);
}

void add_to_cart_button_clicked(GtkWidget *widget, gpointer data) {
        char filename[MAX_FILENAME_LENGTH];
        char product_name[20];
        FILE *fp;

        gchar *name = gtk_label_get_text(GTK_LABEL(data));
        sprintf(filename,"./products/%s.txt", name);

        fp = fopen(filename, "r");
        if (fp == NULL) {
            printf("Failed to open file: %s\n", filename);
        }

        fscanf(fp, "Nom du produit : %[^\n]", product_name);
        printf("Product name : %s\n", product_name);
        fclose(fp);

        FILE *cart_fp = fopen("cart.txt", "a");
        if (cart_fp == NULL) {
            g_print("Failed to open cart file.\n");
            return;
        }

        // Write the product name to the cart file
        fprintf(cart_fp, "Product name : %s\n", product_name);

        // Close the cart file
        fclose(cart_fp);

        g_print("Product added to cart successfully.\n");
    }

void client_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *window = data;
    GtkWidget *dialog;
    GtkWidget *scrolled_window;
    GtkWidget *content_area;
    GtkWidget *grid;
    GtkWidget *name_label, *price_label, *origin_label;

    FILE *fp;
    char product_name[20], price[20], origin[50];
    char filename[MAX_FILENAME_LENGTH];
    DIR *dir;
    struct dirent *ent;
    double prix_total = 0;
    int quantity = 0;
    time_t rawtime;
    struct tm *timeinfo;
    char time_str[80];

    dialog = gtk_dialog_new_with_buttons("Client", GTK_WINDOW(window), GTK_DIALOG_MODAL, "Buy", GTK_RESPONSE_ACCEPT, "Cancel", GTK_RESPONSE_CANCEL, NULL);

    // Create a scrolled window to contain the dialog content
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolled_window, 500, 600); // Set height to 300 pixels
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_set_border_width(GTK_CONTAINER(scrolled_window), 5);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_add(GTK_CONTAINER(content_area), scrolled_window);

    grid = gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_add(GTK_CONTAINER(scrolled_window), grid);

    if ((dir = opendir(PRODUCTS_DIRECTORY)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strcmp((*ent).d_name, ".") != 0 && strcmp((*ent).d_name, "..") != 0) {
                sprintf(filename,"./products/%s", (*ent).d_name);

                fp = fopen(filename, "r");
                if (fp == NULL) {
                    g_print("Failed to open file: %s\n", filename);
                    continue;
                }

                fscanf(fp, "Nom du produit : %[^\n]\nPrix du produit : %s\nProvenance du produit : %[^\n]", product_name, price,origin);
                printf("\n\n Nom du produit : %s\nPrix du produit : %s\n provenance du produit : %s", product_name, price,origin);
                fclose(fp);

                name_label = gtk_label_new("                 Nom du produit :                                                ");
                GtkWidget *name_value = gtk_label_new(product_name);
                price_label = gtk_label_new("                Prix du produit :                                                ");
                GtkWidget *price_value = gtk_label_new(price);
                origin_label = gtk_label_new("                    Provenance du produit :                                                ");
                GtkWidget *origin_value = gtk_label_new(origin);
                GtkWidget *buy_button = gtk_button_new_with_label("Add to cart");

                // Connect the buy button click signal to the add_to_cart_button_clicked function
                g_signal_connect(buy_button, "clicked", G_CALLBACK(add_to_cart_button_clicked), name_value);

                gtk_grid_attach(GTK_GRID(grid), name_label, 0, quantity, 1, 1);
                gtk_grid_attach(GTK_GRID(grid), name_value, 1, quantity, 1, 1);
                gtk_grid_attach(GTK_GRID(grid), price_label, 0, quantity + 1, 1, 1);
                gtk_grid_attach(GTK_GRID(grid), price_value, 1, quantity + 1, 1, 1);
                gtk_grid_attach(GTK_GRID(grid), origin_label, 0, quantity + 2, 1, 1);
                gtk_grid_attach(GTK_GRID(grid), origin_value, 1, quantity + 2, 1, 1);
                gtk_grid_attach(GTK_GRID(grid), buy_button, 0, quantity + 3, 2, 1);

                quantity += 4;
            }
        }
        closedir(dir);
        gtk_widget_show_all(dialog);
    } else {
        g_print("Failed to open directory.\n");
        return;
    }

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));

    if (result == GTK_RESPONSE_ACCEPT) {
        // Buy button clicked
        FILE *cart_fp = fopen(CART_FILENAME, "r");
        if (cart_fp == NULL) {
            g_print("Failed to open cart file.\n");
            return;
        }

        FILE *receipt_fp = fopen(RECEIPT_FILENAME, "w");
        if (receipt_fp == NULL) {
            g_print("Failed to open receipt file.\n");
            fclose(cart_fp);
            return;
        }

        fprintf(receipt_fp, "===================================================\n");
        fprintf(receipt_fp, "\tNom de cooperative : Arcayth\n");
        fprintf(receipt_fp, "---------------------------------------------------\n");

        char product_name[MAX_FILENAME_LENGTH];
        while (fscanf(cart_fp, "Product name : %[^\n]\n", product_name) != EOF) {
            sprintf(filename,"./products/%s.txt", product_name);
            fp = fopen(filename, "r");
            if (fp == NULL) {
                g_print("Failed to open file: %s\n", filename);
                continue;
            }

            fscanf(fp, "Nom du produit : %[^\n]\nPrix du produit : %s", product_name, price);
            prix_total += atof(price);
            quantity += 1;
            fprintf(receipt_fp, "\tNom du produit : %s\n\tPrix du produit : %s\n", product_name, price);
            fprintf(receipt_fp, "---------------------------------------------------\n");
            fclose(fp);
        }

        fprintf(receipt_fp, "\tQuantitee : %d\n", quantity);
        fprintf(receipt_fp, "\tPrix total : %.2f\n", prix_total);

        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);

        fprintf(receipt_fp, "---------------------------------------------------\n");
        fprintf(receipt_fp, "\tDate d'achat : %s\n", time_str);
        fprintf(receipt_fp, "---------------------------------------------------\n");
        fprintf(receipt_fp, "Nourisser votre corps, nourissere la planete!\n");
        fprintf(receipt_fp, "===================================================\n");

        fclose(cart_fp);
        fclose(receipt_fp);
        remove(CART_FILENAME);
        g_print("Receipt generated successfully");
    } else {
        // Cancel button clicked
        remove(CART_FILENAME);
        g_print("Cart cleared.\n");
    }

    gtk_widget_destroy(dialog);
}

void review_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *window = data;
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *grid;
    GtkWidget *reviewer_name_label, *review_text_label;
    GtkWidget *reviewer_name_entry, *review_text_entry;
    GtkWidget *button;
    gchar *reviewer_name, *review_text;

    FILE *fp;
    char filename[MAX_FILENAME_LENGTH];

    dialog = gtk_dialog_new_with_buttons("Add review ", NULL, GTK_DIALOG_MODAL, "Save", GTK_RESPONSE_ACCEPT, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 300);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    grid = gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    reviewer_name_label = gtk_label_new("Reviewer name :");
    reviewer_name_entry = gtk_entry_new();
    review_text_label = gtk_label_new("Review text :");
    review_text_entry = gtk_entry_new();

    gtk_container_add(GTK_CONTAINER(content_area), reviewer_name_label);
    gtk_container_add(GTK_CONTAINER(content_area), reviewer_name_entry);
    gtk_container_add(GTK_CONTAINER(content_area), review_text_label);
    gtk_container_add(GTK_CONTAINER(content_area), review_text_entry);

    gtk_widget_show_all(dialog);

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));

    if (result == GTK_RESPONSE_ACCEPT) {
        reviewer_name = gtk_entry_get_text(GTK_ENTRY(reviewer_name_entry));
        review_text = gtk_entry_get_text(GTK_ENTRY(review_text_entry));

        if (strlen(reviewer_name) == 0 || strlen(review_text) == 0) {
            printf("Make sure to type in valid info.\n");
            gtk_widget_destroy(dialog);
            return;
        }

        sprintf(filename,"./reviews/%s.txt", reviewer_name);

        fp = fopen(filename, "w");
        if (fp == NULL) {
            g_print("Failed to open file.\n");
            return;
        }
        fprintf(fp, "Reviewer name : %s\nReview text : %s", reviewer_name, review_text);

        fclose(fp);
        g_print("Review information saved successfully.\n");
    }

    gtk_widget_destroy(dialog);
}

void check_reviews_button(GtkWidget *widget, gpointer data) {
    GtkWidget *window = data;
    GtkWidget *dialog;
    GtkWidget *scrolled_window;
    GtkWidget *content_area;
    GtkWidget *grid;
    GtkWidget *reviewer_name_label, *review_text_label;

    int counter;
    gchar *reviewer_name, *review_text;
    FILE *fp;
    char product_name[20], price[20], origin[50];
    char filename[MAX_FILENAME_LENGTH];
    DIR *dir;
    struct dirent *ent;

    dialog = gtk_dialog_new_with_buttons("Client", GTK_WINDOW(window), GTK_DIALOG_MODAL, "Cancel", GTK_RESPONSE_CANCEL, NULL);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolled_window, 500, 600); // Set height to 300 pixels
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_set_border_width(GTK_CONTAINER(scrolled_window), 5);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_add(GTK_CONTAINER(content_area), scrolled_window);

    grid = gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_add(GTK_CONTAINER(scrolled_window), grid);

    if ((dir = opendir(PRODUCTS_DIRECTORY)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strcmp((*ent).d_name, ".") != 0 && strcmp((*ent).d_name, "..") != 0) {
                sprintf(filename,"reviews/%s", (*ent).d_name);

                fp = fopen(filename, "r");
                if (fp == NULL) {
                    g_print("Failed to open file: %s\n", filename);
                    continue;
                }

                fscanf(fp, "Reviewer name : %s\nReview text : %s", reviewer_name, review_text);
                fclose(fp);

                reviewer_name_label = gtk_label_new("                 Reviewer name :                                                ");
                GtkWidget *reviewer_name = gtk_label_new(product_name);
                review_text_label = gtk_label_new("                Review text :                                                ");
                GtkWidget *review_text = gtk_label_new(price);

                gtk_grid_attach(GTK_GRID(grid), reviewer_name_label, 0, counter, 1, 1);
                gtk_grid_attach(GTK_GRID(grid), reviewer_name, 1, counter, 1, 1);
                gtk_grid_attach(GTK_GRID(grid), review_text_label, 0, counter + 1, 1, 1);
                gtk_grid_attach(GTK_GRID(grid), review_text, 1, counter + 1, 1, 1);

                counter += 2;
            }
        }
        closedir(dir);
        gtk_widget_show_all(dialog);
    } else {
        g_print("Failed to open directory.\n");
        return;
    }
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *title;
    GtkWidget *seller_button, *client_button, *add_review_button, *view_reviews_button;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Product Manager");
    gtk_container_set_border_width(GTK_CONTAINER(window), 20);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_container_add(GTK_CONTAINER(window), grid);

    title = gtk_label_new("Boujour et Bienvenue dans notre application ! \nComment voulez-vous nous rejoindre ?\n");
    gtk_label_set_justify(GTK_LABEL(title), GTK_JUSTIFY_CENTER);
    gtk_grid_attach(GTK_GRID(grid), title, 0, 0, 2, 1);

    seller_button = gtk_button_new_with_label("Employee dans la cooperative.");
    g_signal_connect(seller_button, "clicked", G_CALLBACK(seller_button_clicked), window);
    gtk_grid_attach(GTK_GRID(grid), seller_button, 0, 1, 1, 1);

    client_button = gtk_button_new_with_label("           Un client         ");
    g_signal_connect(client_button, "clicked", G_CALLBACK(client_button_clicked), window);
    gtk_grid_attach(GTK_GRID(grid), client_button, 1, 1, 1, 1);

    add_review_button = gtk_button_new_with_label("Add review");
    g_signal_connect(add_review_button, "clicked", G_CALLBACK(review_button_clicked), window);
    gtk_grid_attach(GTK_GRID(grid), add_review_button, 0, 2, 1, 1);

    view_reviews_button = gtk_button_new_with_label("Check review");
    g_signal_connect(view_reviews_button, "clicked", G_CALLBACK(check_reviews_button), window);
    gtk_grid_attach(GTK_GRID(grid), view_reviews_button, 1, 2, 1, 1);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
