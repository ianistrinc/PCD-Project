#include <opencv2/opencv.hpp>

int main(int argc, char **argv)
{
    // Verifică dacă calea imaginii este furnizată ca argument
    if (argc != 2)
    {
        printf("Utilizare: %s <cale_imagine>\n", argv[0]);
        return -1;
    }

    // Încarcă imaginea
    cv::Mat src = cv::imread(argv[1], cv::IMREAD_COLOR);
    if (src.empty())
    {
        printf("Nu s-a putut încărca imaginea: %s\n", argv[1]);
        return -1;
    }
    
    // Convertirea imaginii în gri
    cv::Mat gray;
    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);

    // Aplicarea filtrului Gaussian pentru a reduce zgomotul și pentru a evita detectările false de contururi
    cv::GaussianBlur(gray, gray, cv::Size(5, 5), 1.5);

    // Aplicarea detectării muchiilor folosind algoritmul Canny
    cv::Mat edges;
    cv::Canny(gray, edges, 100, 200);

    // Găsirea contururilor
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Desenarea contururilor pe imaginea originală
    cv::Mat result = src.clone();
    cv::drawContours(result, contours, -1, cv::Scalar(0, 255, 0), 2);

    // Afișarea imaginilor
    cv::imshow("Original", src);
    cv::imshow("Contururi", result);

    // Așteaptă să se apese o tastă
    cv::waitKey(0);

    return 0;
}
