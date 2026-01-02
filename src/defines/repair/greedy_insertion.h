#ifndef GREEDY_INSERTION_H
#define GREEDY_INSERTION_H

#include "../alns_definitions.h"
// HATA ÇÖZÜMÜ: 3 klasör geri çıkarak kütüphaneye ulaşan doğru yol
#include "../../../libraries/adaptive-large-neighbourhood-search/src/RepairMethod.h"
#include <vector>
#include <limits>
#include <memory>
#include <algorithm>

class GreedyInsertion : public mlpalns::RepairMethod<PDPTWT_solution> {
public:
    // ALNS kütüphanesi için zorunlu olan kopya oluşturma (clone) fonksiyonu
    std::unique_ptr<mlpalns::RepairMethod<PDPTWT_solution>> clone() const override {
        return std::make_unique<GreedyInsertion>(*this);
    }

    void repair_solution(PDPTWT_solution& solution, std::mt19937& mt) override {
        (void)mt; // Kullanılmayan parametre uyarısını kapatır

        // Dışarıda (unassigned) hiçbir request kalmayana kadar döngüye gir
        while (true) {
            int best_req = -1;
            int best_route = -1;
            double min_insertion_cost = std::numeric_limits<double>::max();

            // 1. Her bir atanmamış request'i tara
            for (int i = 0; i < (int)solution.unassigned.size(); i++) {
                if (solution.unassigned[i] == 1) { // 1 = Dışarıda/Atanmamış
                    
                    // 2. Bu request için tüm araç rotalarını dene
                    for (int r = 0; r < (int)solution.routes.size(); r++) {
                        
                        // ÖNEMLİ: Burada request i'yi rota r'ye eklemenin maliyetini hesaplamalısın.
                        // Şimdilik derleme hatasını geçmek için 0.0 bırakıyorum.
                        // Buraya ileride: double current_delta = solution.calculate_insertion_cost(i, r);
                        double current_delta = 0.0; 
                        
                        if (current_delta < min_insertion_cost) {
                            min_insertion_cost = current_delta;
                            best_req = i;
                            best_route = r;
                        }
                    }
                }
            }

            // 3. Bulunan en ucuz/en iyi request'i rotaya dahil et
            if (best_req != -1) {
                // Not: solution içinde 'insert_request' gibi bir fonksiyonun olduğunu varsayıyoruz
                // solution.insert_request(best_req, best_route); 
                solution.unassigned[best_req] = 0; // Atandı olarak işaretle
            } else {
                break; // Yerleştirilecek uygun yer kalmadıysa çık
            }
        }
    }
};

#endif