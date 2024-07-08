#pragma once
#include "Core/Buffer.h"

#include <glm.hpp>
#include <memory>
#include <vector>

template<typename T>
T clamp(T value, T min, T max) {
    return std::max(min, std::min(value, max));
}

template<typename Predicate>
int FindInterval(int size, const Predicate& pred) {
    int first = 0, len = size;
    while (len > 0) {
        int half = len >> 1, middle = first + half;
        // Bisect range based on value of _pred_ at _middle_
        if (pred(middle)) {
            first = middle + 1;
            len -= half + 1;
        } else
            len = half;
    }
    return clamp(first - 1, 0, size - 2);
}

struct Distribution1D {

    Distribution1D(const float* f, int n) : func(f, f + n), cdf(n + 1) {
        // Compute integral of step function at $x_i$
        cdf[0] = 0;
        for (int i = 1; i < n + 1; ++i) cdf[i] = cdf[i - 1] + func[i - 1] / n;

        // Transform step function integral into CDF
        funcInt = cdf[n];
        if (funcInt == 0) {
            for (int i = 1; i < n + 1; ++i) cdf[i] = float(i) / float(n);
        } else {
            for (int i = 1; i < n + 1; ++i) cdf[i] /= funcInt;
        }
    }

    int SampleDiscrete(float sample, float* pdf) const {
        int offset = FindInterval((int)cdf.size(),
                                  [&](int index) { return cdf[index] <= sample; });
        if (pdf) *pdf = DiscretePDF(offset);
        return offset;
    }

    float SampleContinuous(float u, float* pdf, int* off = nullptr) const {
        // Find surrounding CDF segments and _offset_
        int offset = FindInterval((int)cdf.size(),
                                  [&](int index) { return cdf[index] <= u; });
        if (off) *off = offset;
        // Compute offset along CDF segment
        float du = u - cdf[offset];
        if ((cdf[offset + 1] - cdf[offset]) > 0) {
            du /= (cdf[offset + 1] - cdf[offset]);
        }

        // Compute PDF for sampled offset
        if (pdf) *pdf = (funcInt > 0) ? func[offset] / funcInt : 0;
        if (pdf) *pdf = (funcInt > 0) ? cdf[offset + 1] - cdf[offset] : 0;

        // Return $x\in{}[0,1)$ corresponding to sample
        return (offset + du) / Count();
    }

    float DiscretePDF(int index) const {
        if (index >= func.size() || funcInt == 0) {
            return 0;
        }
        return func[index] / (funcInt * Count());
    }

    void warp(float& sample, int& index) {
        index  = FindInterval((int)cdf.size(),
                             [&](int index) { return cdf[index] <= sample; });
        sample = (sample - cdf[index]) / func[index];
    }

    int Count() const {
        return int(func.size());
    }

    std::unique_ptr<Buffer> toGpuBuffer(Device& device) const;
    
    std::vector<float> func, cdf;
    float              funcInt;
};

struct Distribution2D {
public:
    // Distribution2D Public Methods
    Distribution2D(const float* data, int nu, int nv);
    glm::vec2 SampleContinuous(const glm::vec2& u, float* pdf) const {
        float pdfs[2];
        int   v;
        float d1 = pMarginal->SampleContinuous(u[1], &pdfs[1], &v);
        float d0 = pConditionalV[v]->SampleContinuous(u[0], &pdfs[0]);
        if (pdf)
            *pdf = pdfs[0] * pdfs[1];
        return glm::vec2(d0, d1);
    }
    float Pdf(const glm::vec2& p) const {
        int iu = clamp(int(p[0] * pConditionalV[0]->Count()), 0, pConditionalV[0]->Count() - 1);
        int iv = clamp(int(p[1] * pMarginal->Count()), 0, pMarginal->Count() - 1);
        return pConditionalV[iv]->func[iu] / pMarginal->funcInt;
    }

    void warp(glm::vec2& uv, int& row, int& column) const;

private:
    // Distribution2D Private Data
    std::vector<std::unique_ptr<Distribution1D>> pConditionalV;
    std::unique_ptr<Distribution1D>              pMarginal;
};

//class Distribution2D
//{
//    int _w, _h;
//    std::glm::vector<float> _marginalPdf, _marginalCdf;
//    std::glm::vector<float> _pdf;
//    std::glm::vector<float> _cdf;
//public:
//    Distribution2D(std::glm::vector<float> weights, int w, int h)
//            : _w(w), _h(h), _pdf(std::move(weights))
//    {
//        _cdf.resize(_pdf.size() + h);
//        _marginalPdf.resize(h, 0.0f);
//        _marginalCdf.resize(h + 1);
//
//        _marginalCdf[0] = 0.0f;
//        for (int y = 0; y < h; ++y) {
//            int idxP = y*w;
//            int idxC = y*(w + 1);
//
//            _cdf[idxC] = 0.0f;
//            for (int x = 0; x < w; ++x, ++idxP, ++idxC) {
//                _marginalPdf[y] += _pdf[idxP];
//                _cdf[idxC + 1] = _cdf[idxC] + _pdf[idxP];
//            }
//            _marginalCdf[y + 1] = _marginalCdf[y] + _marginalPdf[y];
//        }
//
//        for (int y = 0; y < h; ++y) {
//            int idxP = y*w;
//            int idxC = y*(w + 1);
//            int idxTail = idxC + w;
//
//            float rowWeight = _cdf[idxTail];
//            if (rowWeight < 1e-4f) {
//                for (int x = 0; x < w; ++x, ++idxP, ++idxC) {
//                    _pdf[idxP] = 1.0f/w;
//                    _cdf[idxC] = x/float(w);
//                }
//            } else {
//                for (int x = 0; x < w; ++x, ++idxP, ++idxC) {
//                    _pdf[idxP] /= rowWeight;
//                    _cdf[idxC] /= rowWeight;
//                }
//            }
//            _cdf[idxTail] = 1.0f;
//        }
//
//        float totalWeight = _marginalCdf.back();
//        for (float &p : _marginalPdf)
//            p /= totalWeight;
//        for (float &c : _marginalCdf)
//            c /= totalWeight;
//        _marginalCdf.back() = 1.0f;
//    }
//
//    void warp(glm::vec2 &uv, int &row, int &column) const
//    {
//        row = int(std::distance(_marginalCdf.begin(), std::upper_bound(_marginalCdf.begin(), _marginalCdf.end(), uv.y)) - 1);
//        uv.y = clamp((uv.y - _marginalCdf[row])/_marginalPdf[row], 0.0f, 1.0f);
//        auto rowStart = _cdf.begin() + row*(_w + 1);
//        auto rowEnd = rowStart + (_w + 1);
//        column = int(std::distance(rowStart, std::upper_bound(rowStart, rowEnd, uv.x)) - 1);
//        int idxC = row*(_w + 1) + column;
//        int idxP = row*_w + column;
//        uv.x = clamp((uv.x - _cdf[idxC])/_pdf[idxP], 0.0f, 1.0f);
//    }
//
//
////
////
////        float pdfs[2];
////        int v;
////        float d1 = pMarginal->SampleContinuous(u[1], &pdfs[1], &v);
////        float d0 = pConditionalV[v]->SampleContinuous(u[0], &pdfs[0]);
////        if(pdf)
////        *pdf = pdfs[0] * pdfs[1];
////        return glm::vec2(d0, d1);
////    }
//
//    float pdf(int row, int column) const
//    {
//        row    = clamp(row,    0, _h - 1);
//        column = clamp(column, 0, _w - 1);
//        return _pdf[row*_w + column]*_marginalPdf[row];
//    }
//
//    float Pdf(glm::vec2 uv) const {
//        return pdf(int((1.0f - uv.y)*_h), int(uv.x*_w))*_w*_h;
//    }
//
//    glm::vec2 SampleContinuous(const glm::vec2 &u, float *pdf) const {
//        glm::vec2 newUv(u);
//        int row,column;
//        warp(newUv,row,column);
//        glm::vec2 res((newUv.x + column)/_w, 1.0f - (newUv.y + row)/_h);
//        if(pdf) *pdf = Pdf(res);
//        return res;
//    }
//
//    glm::vec2 unwarp(glm::vec2 uv, int row, int column) const
//    {
//        row    = clamp(row,    0, _h - 1);
//        column = clamp(column, 0, _w - 1);
//        int idxC = row*(_w + 1) + column;
//        int idxP = row*_w + column;
//
//        return glm::vec2(
//                uv.x*_pdf[idxP] + _cdf[idxC],
//                uv.y*_marginalPdf[row] + _marginalCdf[row]
//        );
//    }
//};