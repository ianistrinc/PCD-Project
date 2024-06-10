#include <opencv2/opencv.hpp>

using namespace cv;

int main() {
    // Definirea căii fișierului
    const char* imagine_path = "C:/Users/Darius/Pictures/Screenshots/test3.jpg";

    // 1. Citirea imaginii
    Mat imagine = imread(imagine_path, IMREAD_COLOR);
    if (imagine.empty()) {
        printf("Eroare la încărcarea imaginii.\n");
        return -1;
    }

    // 2. Conversia la gri
    Mat imagine_gri;
    cvtColor(imagine, imagine_gri, COLOR_BGR2GRAY);

    // 3. Aplicarea eroziunii
    Mat imagine_erozata;
    Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
    erode(imagine_gri, imagine_erozata, kernel);

    // 4. Extragerea conturului
    Mat contur;
    subtract(imagine_gri, imagine_erozata, contur);

    // Afișarea rezultatului
    imshow("Contur", contur);
    waitKey(0);

    return 0;
}
