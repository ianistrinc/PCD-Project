#include <opencv2/opencv.hpp>
#include <stdio.h>

/*
    Avem functia extrage_contur care primeste parametrii urmatorii
    imagine_path -> path-ul catre imaginea pe care vrem sa o folosim
    contur -> o imagine contur in care sa salvam rezultatul
    imagine_originala -> in care salvam imaginea pe care o dam la input
*/
void extrage_contur(const char* imagine_path, cv::Mat& contur, cv::Mat& imagine_originala) {
    //citim imaginea
    imagine_originala = cv::imread(imagine_path, cv::IMREAD_COLOR);
    //verificam daca path-ul este valabil, daca nu printam erroarea
    if (imagine_originala.empty()) {
        printf("Eroare in path-ul file-ului!\n"); // erroare printata
        return;
    }

    //imagine gri
    cv::Mat imagine_gri;
    //convertim imaginea in gri si o atribuim catre imagine_gri
    cv::cvtColor(imagine_originala, imagine_gri, cv::COLOR_BGR2GRAY);

    //imagine erozata
    cv::Mat imagine_erozata;
    // avem un kernel pentru imaginea de eroziune
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    // aplicam eroziunea pe imaginea în nuante de gri
    cv::erode(imagine_gri, imagine_erozata, kernel);

    // extragem conturu 
    cv::subtract(imagine_gri, imagine_erozata, contur);
}


/*
    Avem functia intoarce_obiecte care primeste parametrii urmatorii
    contur -> imagine obtinuta prin functia extrage contur
    imagine_originala -> in care salvam imaginea pe care o dam la input
*/
void intoarce_obiecte(const cv::Mat& contur, cv::Mat& imagine_originala) {
    //este un vector de vectori care va stoca contururile gasite in imagine.
    std::vector<std::vector<cv::Point>> contours;
    /*
        RETR_EXTERNAL dorim doar contur exterior/cele mai mari
        CHAIN_APPROX_SIMPLE reduce numarul de puncte stocate, eleminand punctele redundante.
    */
    cv::findContours(contur, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // o clona a imaginii originale pentru obiectele rotite pe care le vom plasa aici
    cv::Mat imagine_noua = imagine_originala.clone();

    // iterare prin contururi 
    for (size_t i = 0; i < contours.size(); ++i) {
        // obtinem bounding box-ul
        cv::Rect bounding_box = cv::boundingRect(contours[i]);
        // extragem regiunea de interes din imaginea originala
        cv::Mat roi = imagine_originala(bounding_box);
        // rotim obiectele cu capul in jos
        cv::Mat roi_rotit;
        cv::rotate(roi, roi_rotit, cv::ROTATE_180);
        // punem inapoi in pozitia corespunzatoare
        roi_rotit.copyTo(imagine_noua(bounding_box));
    }

    // actualizam imaginea originala cu cea noua, cu versiunea finala 
    imagine_originala = imagine_noua;
}

int main() {
    const char* imagine_path = "C:/Users/Darius/Pictures/Screenshots/test.png"; // avem path-ul imagini dorite
    cv::Mat contur, imagine_originala; // stocam imaginea de contur si imaginea originala

    //extragem contur cu ajutorul functiei extrage_contur
    extrage_contur(imagine_path, contur, imagine_originala);

    //verificam daca imaginea contur este procesata
    if (!contur.empty()) {
        //folosim functia intoarce_obiecte pentru a intoarce toate obiectele 
        intoarce_obiecte(contur, imagine_originala);
        //dam display la Imagine
        cv::imshow("Obiecte Rotite", imagine_originala);
        cv::waitKey(0);
    }

    return 0;
}
