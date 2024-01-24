/**
 *  Copyright 2015 Mike Reed
 */

#include "image.h"
#include "../include/GCanvas.h"
#include "../include/GColor.h"
#include "../include/GBitmap.h"
#include <string>

static int pixel_diff(GPixel p0, GPixel p1) {
    int da = abs(GPixel_GetA(p0) - GPixel_GetA(p1));
    int dr = abs(GPixel_GetR(p0) - GPixel_GetR(p1));
    int dg = abs(GPixel_GetG(p0) - GPixel_GetG(p1));
    int db = abs(GPixel_GetB(p0) - GPixel_GetB(p1));
    return std::max(da, std::max(dr, std::max(dg, db)));
}

static double compare(const GBitmap& a, const GBitmap& b, int tolerance, bool verbose) {
    assert(a.width() == b.width());
    assert(a.height() == b.height());

    int total = 0;

    const GPixel* rowA = a.pixels();
    const GPixel* rowB = b.pixels();

    int total_diff = 0;
    int max_diff = 0;

    for (int y = 0; y < a.height(); ++y) {
        for (int x = 0; x < a.width(); ++x) {
            // we don't score transparent pixels if both a and b are transparent (background)
            if (!rowA[x] && !rowB[x]) {
                continue;
            }

            int diff = pixel_diff(rowA[x], rowB[x]) - tolerance;
            if (diff > 0) {
                total_diff += diff;
                max_diff = std::max(max_diff, diff);
            }
            total += 255;
        }
        rowA = (const GPixel*)((const char*)rowA + a.rowBytes());
        rowB = (const GPixel*)((const char*)rowB + b.rowBytes());
    }

    double score = 1.0 * (total - total_diff) / total;
    assert(score >= 0 && score <= 1);
    score *= score;
    if (verbose) {
        printf(" score %3d", (int)(score * 100));
    }
    return score;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

static void handle_proc(const GDrawRec& rec, const char path[], GBitmap* bitmap) {
    bitmap->alloc(rec.fWidth, rec.fHeight);

    auto canvas = GCreateCanvas(*bitmap);
    if (!canvas) {
        fprintf(stderr, "failed to create canvas for [%d %d] %s\n",
                rec.fWidth, rec.fHeight, rec.fName);
        return;
    }

    canvas->clear({0, 0, 0, 0});
    rec.fDraw(canvas.get());

    if (!bitmap->writeToFile(path)) {
        fprintf(stderr, "failed to write %s\n", path);
    }
}

static bool is_arg(const char arg[], const char name[]) {
    std::string str("--");
    str += name;
    if (!strcmp(arg, str.c_str())) {
        return true;
    }

    char shortVers[3];
    shortVers[0] = '-';
    shortVers[1] = name[0];
    shortVers[2] = 0;
    return !strcmp(arg, shortVers);
}

static void add_image(FILE* f, const char path[], const char name[], const char suffix[],
                      const GBitmap& bm) {
    std::string str(name);
    str += "__";
    str += suffix;
    str += ".png";
    fprintf(f, "<a href=\"%s\"><img src=\"%s\" /></a>\n", str.c_str(), str.c_str());

    std::string full(path);
    full += "/";
    full += str;
    bm.writeToFile(full.c_str());
}

static void add_diff_to_file(FILE* f, const GBitmap& test, const GBitmap& orig, const char path[],
                             const char name[]) {
    const int w = test.width();
    const int h = test.height();
    GBitmap diff0, diff1;
    diff0.alloc(w, h);
    diff1.alloc(w, h);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int diff = pixel_diff(*test.getAddr(x, y), *orig.getAddr(x, y));
            *diff0.getAddr(x, y) = GPixel_PackARGB(0xFF, diff, diff, diff);
            if (diff > 0) {
                diff = 0xFF;
            }
            *diff1.getAddr(x, y) = GPixel_PackARGB(0xFF, diff, diff, diff);
        }
    }

    fprintf(f, "%s<br/>\n", name);
    add_image(f, path, name, "test", test); fprintf(f, "&nbsp;&nbsp;");
    add_image(f, path, name, "orig", orig); fprintf(f, "&nbsp;&nbsp;");
    add_image(f, path, name, "dif0", diff0); fprintf(f, "&nbsp;&nbsp;");
    add_image(f, path, name, "dif1", diff1); fprintf(f, "<br><br>\n");
}

static int gPACounts[10] = { 0,0,0,0,0,0,0,0,0,0 };
static int gDrawCount;

static int max_name_len() {
    size_t len = 0;
    for (int i = 0; gDrawRecs[i].fDraw; ++i) {
        len = std::max(len, strlen(gDrawRecs[i].fName));
    }
    return (int)len;
}

int main_image(int argc, const char* argv[]) {
    bool verbose = false;
    std::string root;
    const char* match = NULL;
    const char* expected = NULL;
    const char* diffDir = NULL;
    const char* scoreFile = nullptr;
    FILE* diffFile = NULL;
    int tolerance = 0;

    const char* collage_dir = nullptr;
    int collage_index = -1;
    FILE* collage_file = nullptr;

    for (int i = 0; gDrawRecs[i].fDraw; ++i) {
        assert((unsigned)gDrawRecs[i].fPA < GARRAY_COUNT(gPACounts));
        gPACounts[gDrawRecs[i].fPA] += 1;
        gDrawCount += 1;
    }

    for (int i = 1; i < argc; ++i) {
        if (is_arg(argv[i], "verbose")) {
            verbose = true;
        } else if (is_arg(argv[i], "write") && i+1 < argc) {
            root = argv[++i];
        } else if (is_arg(argv[i], "match") && i+1 < argc) {
            match = argv[++i];
        } else if (is_arg(argv[i], "expected") && i+1 < argc) {
            expected = argv[++i];
        } else if (is_arg(argv[i], "tolerance") && i+1 < argc) {
            tolerance = atoi(argv[++i]);
            assert(tolerance >= 0);
        } else if (is_arg(argv[i], "scoreFile") && i+1 < argc) {
            scoreFile = argv[++i];
        } else if (is_arg(argv[i], "diff") && i+1 < argc) {
            diffDir = argv[++i];
            std::string path(diffDir);
            path += "/index.html";
            diffFile = fopen(path.c_str(), "w");
            if (!diffFile) {
                printf("------- failed to create %s\n", path.c_str());
            } else {
                fprintf(diffFile, "<h3>Test Orig Diff DIFF</h3>\n");
            }
        } else if (is_arg(argv[i], "collage") && i+2 < argc) {
            collage_dir = argv[++i];
            collage_index = atol(argv[++i]);
            std::string path(collage_dir);
            path += "/index.html";
            collage_file = fopen(path.c_str(), "a");
            if (!collage_file) {
                printf("------- collage file failed\n");
                printf("file: >%s<\n", path.c_str());
                collage_dir = nullptr;
            }
        }
    }

    if (root.size() > 0 && root[root.size() - 1] != '/') {
        root += "/";
        if (!mk_dir(root.c_str())) {
            return -1;
        }
    }

    // pa#_NAME.png -- so add 8 to the name length for the total
    const int maxNameLen = max_name_len() + 8;

    double percent_correct = 0;
    double counter = 0;
    for (int i = 0; gDrawRecs[i].fDraw; ++i) {
        double weight = 1 << (gDrawRecs[i].fPA - 1);
        weight /= gPACounts[gDrawRecs[i].fPA];

        std::string path(root);
        path += gDrawRecs[i].fName;
        path += ".png";

        bool something = strncmp(gDrawRecs[i].fName, "something_", strlen("something_")) == 0;
        if (!something) {
            counter += weight;
        }

        if (match && !strstr(path.c_str(), match)) {
            continue;
        }

        if (verbose && !something) {
            printf("image: [%2d] %*s", i, maxNameLen, path.c_str());
        }
        
        GBitmap testBM;
        handle_proc(gDrawRecs[i], path.c_str(), &testBM);

        if (expected && !something) {
            std::string exp_path(expected);
            exp_path += "/";
            exp_path += gDrawRecs[i].fName;
            exp_path += ".png";
            GBitmap expectedBM;
    
            if (!expectedBM.readFromFile(exp_path.c_str())) {
                printf("- failed to load <%s>", exp_path.c_str());
            } else {
                double correct = compare(testBM, expectedBM, tolerance, verbose);
                if (correct < 1 && diffFile != NULL) {
                    add_diff_to_file(diffFile, testBM, expectedBM, diffDir, gDrawRecs[i].fName);
                }
                double individual_score = correct * weight;

                percent_correct += individual_score;
            }
        }

        if (verbose && !something) {
            printf("\n");
        }

        free(testBM.pixels());
    }
    if (diffFile) {
        fclose(diffFile);
    }

    constexpr int num_required = 2;

    double dscore = percent_correct * 100 / counter;
    int image_score = std::min((int)(dscore * num_required + 0.5), 100);

    if (expected) {
        printf("          final: %3d\n", image_score);
    }
    if (scoreFile) {
        FILE* f = fopen(scoreFile, "w");
        if (f) {
            fprintf(f, "%d", image_score);
            fclose(f);
        } else {
            printf("FAILED TO WRITE TO %s\n", scoreFile);
            return -1;
        }
    }
    return 0;
}
