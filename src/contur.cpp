#include "opencv2/opencv.hpp"

int main(int argc, char* argv[]) {
    // Verificăm dacă avem numărul corect de argumente
    if (argc != 2) {
        printf("Utilizare: %s <cale_imagine>\n", argv[0]);
        return -1;
    }

    // alegem path-ul unde se afla imaginea din argument
    const char* imagine_path = argv[1];

    // citim imaginea
    cv::Mat imagine = cv::imread(imagine_path, cv::IMREAD_COLOR); // specificam ca imaginea trebuie citita RGB

    // verificam daca path-ul este valid
    if (imagine.empty()) {
        // printam erroarea
        printf("Path-ul imaginii nu este valid!\n");
        return -1;
    }

    // adaugam o alta imagine pentru a o converti in gri
    cv::Mat imagine_gri;
    // convertim imaginea in gri si o atribuim catre imagine_gri
    cv::cvtColor(imagine, imagine_gri, cv::COLOR_BGR2GRAY);

    // adaugam o imagine erodata
    cv::Mat imagine_erozata;
    // adaugam un kernel pentru imaginea de eroziune
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)); // matrice dreptunghiulara de 3x3
    // aplicam eroziunea pe imaginea în nuante de gri
    cv::erode(imagine_gri, imagine_erozata, kernel);

    // extragem conturu 
    cv::Mat contur;
    // scadem imaginea erodata din imaginea gri, acest lucru pune in evidenta conturul din imagine
    cv::subtract(imagine_gri, imagine_erozata, contur);

    // afisam rezultatele
    cv::imshow("Contur", contur);
    cv::waitKey(0);

    return 0;
}
