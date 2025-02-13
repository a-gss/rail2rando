#pragma once

bool regex_wrapper(const char *str, const char *pattern,
                   int n_capturing_group, char result[][64])
{
    regex_t regex;
    regmatch_t matches[n_capturing_group + 1];  // Index 0 = match complet

    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        error("Erreur de compilation du regex");
        return false;
    }

    if (regexec(&regex, str, n_capturing_group + 1, matches, 0) == 0) {
        if (n_capturing_group == 0) {
            // Si aucun groupe capturant, récupérer le match complet
            int debut = matches[0].rm_so;
            int fin = matches[0].rm_eo;
            int len = fin - debut;

            strncpy(result[0], str + debut, len);
            result[0][len] = '\0';
            printf(GREEN "MATCH: %s\n" RESET, result[0]);

        } else { // Boucle sur les groupes capturés
            printf(GREEN "MATCH: ");
            for (int i = 1; i <= n_capturing_group; i++) {
                if (matches[i].rm_so != -1) { // Vérifie si le groupe est capturé
                    int debut = matches[i].rm_so;
                    int fin = matches[i].rm_eo;
                    int len = fin - debut;

                    strncpy(result[i-1], str + debut, len);
                    result[i-1][len] = '\0';

                    (n_capturing_group > 1) ? printf("(%d)%s ", i, result[i-1])
                                            : printf("%s ", result[i-1]);
                }
            }
            puts(RESET);
            regfree(&regex);
            return true;
        }

    } else {
        regfree(&regex);
        return false;
    }
}

bool regex_find_mmaped(const char *pattern, char *mmaped_data,
                       int n_capturing_group, char result[][64])
{
    int i = 0;
    int j = 0;
    char line[256] = {};
    while(mmaped_data[i] != '\0') {
        if (mmaped_data[i] == '\r') {           // saloperie de windows de merde
            i++;
            continue;
        }

        if (mmaped_data[i] == '\n') {           // end of line
            line[j] = '\0';                     // null terminate the line
            if (regex_wrapper(line, pattern, n_capturing_group, result) == true) {
                return true;
            } else {
                i++;                            // move to the next line
                j = 0;
                //memset(line, 0, sizeof(line));  // reset the line
                //line[0] = '\0';
                // TODO: a-t-on vraiment besoin de reset la ligne ? ...
                continue;
            }
        } else {
            line[j] = mmaped_data[i];           // store the character
            i++;
            j++;

            if (j >= sizeof(line) - 1) {        // Prevent buffer overflow
                error("Mon gars ça overflow de malade là");
                exit(EXIT_FAILURE);
            }
        }
    }
    puts(RED "NO MATCH" RESET);
    return false;
}
