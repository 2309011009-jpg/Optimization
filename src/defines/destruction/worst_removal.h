#ifndef WORST_REMOVAL_H
#define WORST_REMOVAL_H

#include "../alns_definitions.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <memory>

class WorstRemoval : public mlpalns::DestroyMethod<PDPTWT_solution> {
public:
    std::unique_ptr<mlpalns::DestroyMethod<PDPTWT_solution>> clone() const override {
        return std::make_unique<WorstRemoval>(*this);
    }

    void destroy_solution(PDPTWT_solution& solution, std::mt19937& mt) override {
        // Parametreler: q (çıkarılacak adet), p (rastgelelik şiddeti)
        int q = 3; 
        double p = 3.0; 
        
        struct RequestCost {
            int id;
            double delta;
        };
        std::vector<RequestCost> cost_list;

        // 1. Mevcut toplam maliyeti al
        double base_cost = solution.getCost();

        // 2. Her atanmış request için maliyet kazancını (delta) hesapla
        for (int i = 0; i < (int)solution.unassigned.size(); ++i) {
            if (solution.unassigned[i] == 0) { // Sadece rotada olanlar
                
                // --- KRİTİK KISIM: Simülasyon ---
                // Bu isteği geçici olarak unassigned yapıyoruz
                solution.unassigned[i] = 1; 
                
                // Yeni maliyeti hesapla (İstek yokken maliyet ne oluyor?)
                double cost_without_i = solution.getCost();
                
                // Eski maliyet ile yeni maliyet arasındaki fark (Kazancımız)
                double delta_i = base_cost - cost_without_i;
                
                // İsteği geri "atanmış" durumuna getir (Simülasyon bitti)
                solution.unassigned[i] = 0;

                RequestCost rc;
                rc.id = i;
                rc.delta = delta_i;
                cost_list.push_back(rc);
            }
        }

        if (cost_list.empty()) return;

        // 3. Kazanca (delta) göre büyükten küçüğe sırala (En çok kazandıran en kötüdür)
        std::sort(cost_list.begin(), cost_list.end(), [](const RequestCost& a, const RequestCost& b) {
            return a.delta > b.delta;
        });

        // 4. q kadar request seç ve GERÇEKTEN çıkar
        for (int i = 0; i < q && !cost_list.empty(); ++i) {
            std::uniform_real_distribution<double> dist(0.0, 1.0);
            double r = dist(mt);
            
            // Powell dağılımı mantığıyla index seçimi (p=3 en kötülere odaklanır)
            int index = static_cast<int>(std::pow(r, p) * cost_list.size());
            if (index >= (int)cost_list.size()) index = (int)cost_list.size() - 1;

            int target_id = cost_list[index].id;
            
            // KALICI OLARAK ÇIKAR
            solution.unassigned[target_id] = 1; 
            
            // Listeyi güncelle
            cost_list.erase(cost_list.begin() + index);
        }
    }
};

#endif