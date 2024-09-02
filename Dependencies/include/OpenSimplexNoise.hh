/*
 * OpenSimplex (Simplectic) Noise in C++
 * by Arthur Tombs
 *
 * Modified 2015-01-08
 *
 * This is a derivative work based on OpenSimplex by Kurt Spencer:
 *   https://gist.github.com/KdotJPG/b1270127455a94ac5d19
 *
 * Anyone is free to make use of this software in whatever way they want.
 * Attribution is appreciated, but not required.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef OPENSIMPLEXNOISE_HH
#define OPENSIMPLEXNOISE_HH

#include <cmath>

#if __cplusplus < 201103L
#pragma message("Info: Your compiler does not claim C++11 support. Some features may be unavailable.")
#else
#define OSN_USE_CSTDINT
#define OSN_USE_STATIC_ASSERT
#endif

#ifdef OSN_USE_CSTDINT
 // cstdint is required for the int64_t type
#include <cstdint>
#else
#pragma message("Info: Not using <cstdint> for fixed-width integral types. To enable this feature, define OSN_USE_CSTDINT before including this header.")
 // cstdlib is required for the srand and rand functions
#include <cstdlib>
#endif

#ifdef OSN_USE_STATIC_ASSERT
  // type_traits is required for the is_floating_point function
#include <type_traits>
#endif


namespace OSN {

#ifdef OSN_USE_CSTDINT
    typedef uint_fast8_t OSN_BYTE;
#ifndef OSN_INT_TYPE
#define OSN_INT_TYPE int64_t
#endif
#else
    typedef unsigned char OSN_BYTE;
#ifndef OSN_INT_TYPE
#define OSN_INT_TYPE long
#endif
#endif

    typedef OSN_INT_TYPE inttype;

    namespace {

        // This function seems to be faster than std::pow(x, 4) in all cases
        template <typename T>
        inline T pow4(T x) {
            x *= x;
            return x * x;
        }

        template <typename T>
        inline T pow2(T x) {
            return x * x;
        }

        template <typename T>
        inline inttype fastFloori(T x) {
            inttype ip = (inttype)x;
#ifndef OSN_ALWAYS_POSITIVE
            if (x < 0.0) --ip;
#endif
            return ip;
        }
    }

    class NoiseBase {

    protected:

        int perm[256];

        // Empty constructor to allow child classes to set up perm themselves.
        NoiseBase(void) {}

#ifdef OSN_USE_CSTDINT
        // Perform one step of the Linear Congruential Generator algorithm.
        inline static void LCG_STEP(int64_t& x) {
            // Magic constants are attributed to Donald Knuth's MMIX implementation.
            static const int64_t MULTIPLIER = 6364136223846793005LL;
            static const int64_t INCREMENT = 1442695040888963407LL;
            x = ((x * MULTIPLIER) + INCREMENT);
        }

        // Initializes the class using a permutation array generated from a 64-bit seed.
        // Generates a proper permutation (i.e. doesn't merely perform N successive
        // pair swaps on a base array).
        // Uses a simple 64-bit LCG.
        NoiseBase(int64_t seed) {
            int source[256];
            for (int i = 0; i < 256; ++i) { source[i] = i; }
            LCG_STEP(seed);
            LCG_STEP(seed);
            LCG_STEP(seed);
            for (int i = 255; i >= 0; --i) {
                LCG_STEP(seed);
                int r = (int)((seed + 31) % (i + 1));
                if (r < 0) { r += (i + 1); }
                perm[i] = source[r];
                source[r] = source[i];
            }
        }
#else
        // Initializes the class using a permutation array generated from a 32-bit seed.
        // Generates a proper permutation (i.e. doesn't merely perform N successive
        // pair swaps on a base array).
        NoiseBase(long seed) {
            int source[256];
            for (int i = 0; i < 256; ++i) { source[i] = i; }
            srand(seed);
            for (int i = 255; i >= 0; --i) {
                int r = (int)(rand() % (i + 1));
                perm[i] = source[r];
                source[r] = source[i];
            }
        }
#endif

        NoiseBase(const int* p) {
            // Copy the supplied permutation array into this instance
            for (int i = 0; i < 256; ++i) { perm[i] = p[i]; }
        }

    };


    template <int N>
    class Noise : public NoiseBase {
    };

    // 2D Implementation of the OpenSimplexNoise generator.
    template <>
    class Noise <2> : public NoiseBase {
    private:

        static const int gradients[16];

        template <typename T>
        inline T extrapolate(inttype xsb, inttype ysb, T dx, T dy) const {
            unsigned int index = perm[(perm[xsb & 0xFF] + ysb) & 0xFF] & 0x0E;
            return gradients[index] * dx +
                gradients[index + 1] * dy;
        }

        template <typename T>
        inline T extrapolate(inttype xsb, inttype ysb, T dx, T dy, T(&v)[2]) const {
            unsigned int index = perm[(perm[xsb & 0xFF] + ysb) & 0xFF] & 0x0E;
            return (v[0] = gradients[index]) * dx +
                (v[1] = gradients[index + 1]) * dy;
        }

    public:

#ifdef OSN_USE_CSTDINT
        Noise(int64_t seed = 0LL) : NoiseBase(seed) {}
#else
        Noise(long seed = 0L) : NoiseBase(seed) {}
#endif
        Noise(const int* p) : NoiseBase(p) {}


        template <typename T>
        T eval(T x, T y) const {

#ifdef OSN_USE_STATIC_ASSERT
            static_assert(std::is_floating_point<T>::value, "OpenSimplexNoise can only be used with floating-point types");
#endif

            static const T STRETCH_CONSTANT = (T)((1.0 / std::sqrt(2.0 + 1.0) - 1.0) * 0.5);
            static const T SQUISH_CONSTANT = (T)((std::sqrt(2.0 + 1.0) - 1.0) * 0.5);
            static const T NORM_CONSTANT = (T)(1.0 / 47.0);

            inttype xsb, ysb, xsv_ext, ysv_ext;
            T dx0, dy0, dx_ext, dy_ext;
            T xins, yins;

            // Parameters for the four contributions
            T contr_m[4], contr_ext[4];

            {
                // Place input coordinates on a grid.
                T stretchOffset = (x + y) * STRETCH_CONSTANT;
                T xs = x + stretchOffset;
                T ys = y + stretchOffset;

                // Floor to get grid coordinates of rhombus super-cell origin.
#ifdef __FAST_MATH__
                T xsbd = std::floor(xs);
                T ysbd = std::floor(ys);
                xsb = (inttype)xsbd;
                ysb = (inttype)ysbd;
#else
                xsb = fastFloori(xs);
                ysb = fastFloori(ys);
                T xsbd = (T)xsb;
                T ysbd = (T)ysb;
#endif

                // Skew out to get actual coordinates of rhombohedron origin.
                T squishOffset = (xsbd + ysbd) * SQUISH_CONSTANT;
                T xb = xsbd + squishOffset;
                T yb = ysbd + squishOffset;

                // Positions relative to origin point.
                dx0 = x - xb;
                dy0 = y - yb;

                // Compute grid coordinates relative to rhomboidal origin.
                xins = xs - xsbd;
                yins = ys - ysbd;
            }

            // Contribution (1,0).
            {
                T dx1 = dx0 - (T)1.0 - SQUISH_CONSTANT;
                T dy1 = dy0 - SQUISH_CONSTANT;
                contr_m[0] = pow2(dx1) + pow2(dy1);
                contr_ext[0] = extrapolate(xsb + 1, ysb, dx1, dy1);
            }

            // Contribution (0,1).
            {
                T dx2 = dx0 - SQUISH_CONSTANT;
                T dy2 = dy0 - (T)1.0 - SQUISH_CONSTANT;
                contr_m[1] = pow2(dx2) + pow2(dy2);
                contr_ext[1] = extrapolate(xsb, ysb + 1, dx2, dy2);
            }

            if ((xins + yins) <= (T)1.0) {
                // Inside the triangle (2-Simplex) at (0,0).
                T zins = (T)1.0 - (xins + yins);
                if (zins > xins || zins > yins) {
                    // (0,0) is one of the closest two triangular vertices.
                    if (xins > yins) {
                        xsv_ext = xsb + 1;
                        ysv_ext = ysb - 1;
                        dx_ext = dx0 - (T)1.0;
                        dy_ext = dy0 + (T)1.0;
                    }
                    else {
                        xsv_ext = xsb - 1;
                        ysv_ext = ysb + 1;
                        dx_ext = dx0 + (T)1.0;
                        dy_ext = dy0 - (T)1.0;
                    }
                }
                else {
                    // (1,0) and (0,1) are the closest two vertices.
                    xsv_ext = xsb + 1;
                    ysv_ext = ysb + 1;
                    dx_ext = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    dy_ext = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                }
            }
            else {
                // Inside the triangle (2-Simplex) at (1,1).
                T zins = (T)2.0 - (xins + yins);
                if (zins < xins || zins < yins) {
                    // (0,0) is one of the closest two triangular vertices.
                    if (xins > yins) {
                        xsv_ext = xsb + 2;
                        ysv_ext = ysb;
                        dx_ext = dx0 - (T)2.0 - (SQUISH_CONSTANT * (T)2.0);
                        dy_ext = dy0 - (SQUISH_CONSTANT * (T)2.0);
                    }
                    else {
                        xsv_ext = xsb;
                        ysv_ext = ysb + 2;
                        dx_ext = dx0 - (SQUISH_CONSTANT * (T)2.0);
                        dy_ext = dy0 - (T)2.0 - (SQUISH_CONSTANT * (T)2.0);
                    }
                }
                else {
                    // (1,0) and (0,1) are the closest two vertices.
                    xsv_ext = xsb;
                    ysv_ext = ysb;
                    dx_ext = dx0;
                    dy_ext = dy0;
                }
                xsb += 1;
                ysb += 1;
                dx0 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                dy0 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
            }

            // Contribution (0,0) or (1,1).
            {
                contr_m[2] = pow2(dx0) + pow2(dy0);
                contr_ext[2] = extrapolate(xsb, ysb, dx0, dy0);
            }

            // Extra vertex.
            {
                contr_m[3] = pow2(dx_ext) + pow2(dy_ext);
                contr_ext[3] = extrapolate(xsv_ext, ysv_ext, dx_ext, dy_ext);
            }

            T value = 0.0;
            for (int i = 0; i < 4; ++i) {
                value += pow4(std::max((T)2.0 - contr_m[i], (T)0.0)) * contr_ext[i];
            }

            return (value * NORM_CONSTANT);
        }

        template <typename T>
        void deval(T x, T y, T(&v)[2]) const {

#ifdef OSN_USE_STATIC_ASSERT
            static_assert(std::is_floating_point<T>::value, "OpenSimplexNoise can only be used with floating-point types");
#endif

            static const T STRETCH_CONSTANT = (T)((1.0 / std::sqrt(2.0 + 1.0) - 1.0) * 0.5);
            static const T SQUISH_CONSTANT = (T)((std::sqrt(2.0 + 1.0) - 1.0) * 0.5);
            static const T NORM_CONSTANT = (T)(1.0 / 47.0);

            inttype xsb, ysb, xsv_ext, ysv_ext;
            T dx0, dy0, dx_ext, dy_ext;
            T xins, yins;

            {
                // Place input coordinates on a grid.
                T stretchOffset = (x + y) * STRETCH_CONSTANT;
                T xs = x + stretchOffset;
                T ys = y + stretchOffset;

                // Floor to get grid coordinates of rhombus super-cell origin.
#ifdef __FAST_MATH__
                T xsbd = std::floor(xs);
                T ysbd = std::floor(ys);
                xsb = (inttype)xsbd;
                ysb = (inttype)ysbd;
#else
                xsb = fastFloori(xs);
                ysb = fastFloori(ys);
                T xsbd = (T)xsb;
                T ysbd = (T)ysb;
#endif

                // Skew out to get actual coordinates of rhombohedron origin.
                T squishOffset = (xsbd + ysbd) * SQUISH_CONSTANT;
                T xb = xsbd + squishOffset;
                T yb = ysbd + squishOffset;

                // Positions relative to origin point.
                dx0 = x - xb;
                dy0 = y - yb;

                // Compute grid coordinates relative to rhomboidal origin.
                xins = xs - xsbd;
                yins = ys - ysbd;
            }

            T dv[2] = { 0.0, 0.0 };

            // Contribution (1,0).
            {
                T dx1 = dx0 - (T)1.0 - SQUISH_CONSTANT;
                T dy1 = dy0 - SQUISH_CONSTANT;
                T attn1 = std::max((T)2.0 - ((dx1 * dx1) + (dy1 * dy1)), (T)0.0);
                T de[2];
                T ext = extrapolate(xsb + 1, ysb, dx1, dy1, de);
                dv[0] += pow2(attn1) * (pow2(attn1) * de[0] - ((T)8.0) * attn1 * dx1 * ext);
                dv[1] += pow2(attn1) * (pow2(attn1) * de[1] - ((T)8.0) * attn1 * dy1 * ext);
            }

            // Contribution (0,1).
            {
                T dx2 = dx0 - SQUISH_CONSTANT;
                T dy2 = dy0 - (T)1.0 - SQUISH_CONSTANT;
                T attn2 = std::max((T)2.0 - ((dx2 * dx2) + (dy2 * dy2)), (T)0.0);
                T de[2];
                T ext = extrapolate(xsb, ysb + 1, dx2, dy2, de);
                dv[0] += pow2(attn2) * (pow2(attn2) * de[0] - ((T)8.0) * attn2 * dx2 * ext);
                dv[1] += pow2(attn2) * (pow2(attn2) * de[1] - ((T)8.0) * attn2 * dy2 * ext);
            }

            if ((xins + yins) <= (T)1.0) {
                // Inside the triangle (2-Simplex) at (0,0).
                T zins = (T)1.0 - (xins + yins);
                if (zins > xins || zins > yins) {
                    // (0,0) is one of the closest two triangular vertices.
                    if (xins > yins) {
                        xsv_ext = xsb + 1;
                        ysv_ext = ysb - 1;
                        dx_ext = dx0 - (T)1.0;
                        dy_ext = dy0 + (T)1.0;
                    }
                    else {
                        xsv_ext = xsb - 1;
                        ysv_ext = ysb + 1;
                        dx_ext = dx0 + (T)1.0;
                        dy_ext = dy0 - (T)1.0;
                    }
                }
                else {
                    // (1,0) and (0,1) are the closest two vertices.
                    xsv_ext = xsb + 1;
                    ysv_ext = ysb + 1;
                    dx_ext = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    dy_ext = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                }
            }
            else {
                // Inside the triangle (2-Simplex) at (1,1).
                T zins = (T)2.0 - (xins + yins);
                if (zins < xins || zins < yins) {
                    // (0,0) is one of the closest two triangular vertices.
                    if (xins > yins) {
                        xsv_ext = xsb + 2;
                        ysv_ext = ysb;
                        dx_ext = dx0 - (T)2.0 - (SQUISH_CONSTANT * (T)2.0);
                        dy_ext = dy0 - (SQUISH_CONSTANT * (T)2.0);
                    }
                    else {
                        xsv_ext = xsb;
                        ysv_ext = ysb + 2;
                        dx_ext = dx0 - (SQUISH_CONSTANT * (T)2.0);
                        dy_ext = dy0 - (T)2.0 - (SQUISH_CONSTANT * (T)2.0);
                    }
                }
                else {
                    // (1,0) and (0,1) are the closest two vertices.
                    xsv_ext = xsb;
                    ysv_ext = ysb;
                    dx_ext = dx0;
                    dy_ext = dy0;
                }
                xsb += 1;
                ysb += 1;
                dx0 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                dy0 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
            }

            // Contribution (0,0) or (1,1).
            {
                T attn = std::max((T)2.0 - ((dx0 * dx0) + (dy0 * dy0)), (T)0.0);
                T de[2];
                T ext = extrapolate(xsb, ysb, dx0, dy0, de);
                dv[0] += pow2(attn) * (pow2(attn) * de[0] - ((T)8.0) * attn * dx0 * ext);
                dv[1] += pow2(attn) * (pow2(attn) * de[1] - ((T)8.0) * attn * dy0 * ext);
            }

            // Extra vertex.
            {
                T attn = std::max((T)2.0 - ((dx_ext * dx_ext) + (dy_ext * dy_ext)), (T)0.0);
                T de[2];
                T ext = extrapolate(xsv_ext, ysv_ext, dx_ext, dy_ext, de);
                dv[0] += pow2(attn) * (pow2(attn) * de[0] - ((T)8.0) * attn * dx_ext * ext);
                dv[1] += pow2(attn) * (pow2(attn) * de[1] - ((T)8.0) * attn * dy_ext * ext);
            }

            v[0] = dv[0] * NORM_CONSTANT;
            v[1] = dv[1] * NORM_CONSTANT;
        }

    };

    // Array of gradient values for 2D. They approximate the directions to the
    // vertices of a octagon from its center.
    // Gradient set 2014-10-06.
    const int Noise<2>::gradients[] = {
       5, 2,   2, 5,  -5, 2,  -2, 5,
       5,-2,   2,-5,  -5,-2,  -2,-5
    };


    // 3D Implementation of the OpenSimplexNoise generator.
    template <>
    class Noise <3> : public NoiseBase {
    private:

        // Array of gradient values for 3D. Values are defined below the class definition.
        static const int gradients[72];

        // Because 72 is not a power of two, extrapolate cannot use a bitmask to index
        // into the perm array. Pre-calculate and store the indices instead.
        int permGradIndex[256];

        template <typename T>
        inline T extrapolate(inttype xsb, inttype ysb, inttype zsb, T dx, T dy, T dz) const {
            unsigned int index = permGradIndex[(perm[(perm[xsb & 0xFF] + ysb) & 0xFF] + zsb) & 0xFF];
            return gradients[index] * dx +
                gradients[index + 1] * dy +
                gradients[index + 2] * dz;
        }

        template <typename T>
        inline T extrapolate(inttype xsb, inttype ysb, inttype zsb, T dx, T dy, T dz, T(&de)[3]) const {
            unsigned int index = permGradIndex[(perm[(perm[xsb & 0xFF] + ysb) & 0xFF] + zsb) & 0xFF];
            return (de[0] = gradients[index]) * dx +
                (de[1] = gradients[index + 1]) * dy +
                (de[2] = gradients[index + 2]) * dz;
        }

    public:

#ifdef OSN_USE_CSTDINT
        // Initializes the class using a permutation array generated from a 64-bit seed.
        // Generates a proper permutation (i.e. doesn't merely perform N successive
        // pair swaps on a base array).
        // Uses a simple 64-bit LCG.
        Noise(int64_t seed = 0LL) : NoiseBase() {
            int source[256];
            for (int i = 0; i < 256; ++i) { source[i] = i; }
            LCG_STEP(seed);
            LCG_STEP(seed);
            LCG_STEP(seed);
            for (int i = 255; i >= 0; --i) {
                LCG_STEP(seed);
                int r = (int)((seed + 31) % (i + 1));
                if (r < 0) { r += (i + 1); }
                perm[i] = source[r];
                permGradIndex[i] = (int)((perm[i] % (72 / 3)) * 3);
                source[r] = source[i];
            }
        }
#else
        // Initializes the class using a permutation array generated from a 32-bit seed.
        // Generates a proper permutation (i.e. doesn't merely perform N successive
        // pair swaps on a base array).
        Noise(long seed = 0L) : NoiseBase() {
            int source[256];
            for (int i = 0; i < 256; ++i) { source[i] = i; }
            srand(seed);
            for (int i = 255; i >= 0; --i) {
                int r = (int)(rand() % (i + 1));
                perm[i] = source[r];
                // NB: 72 is the number of elements of the gradients3D array
                permGradIndex[i] = (int)((perm[i] % (72 / 3)) * 3);
                source[r] = source[i];
            }
        }
#endif

        Noise(const int* p) : NoiseBase() {
            // Copy the supplied permutation array into this instance.
            for (int i = 0; i < 256; ++i) {
                perm[i] = p[i];
                permGradIndex[i] = (int)((perm[i] % (72 / 3)) * 3);
            }
        }


        template <typename T>
        T eval(T x, T y, T z) const {

#ifdef OSN_USE_STATIC_ASSERT
            static_assert(std::is_floating_point<T>::value, "OpenSimplexNoise can only be used with floating-point types");
#endif

            static const T STRETCH_CONSTANT = (T)(-1.0 / 6.0); // (1 / sqrt(3 + 1) - 1) / 3
            static const T SQUISH_CONSTANT = (T)(1.0 / 3.0);  // (sqrt(3 + 1) - 1) / 3
            static const T NORM_CONSTANT = (T)(1.0 / 103.0);

            inttype xsb, ysb, zsb;
            T dx0, dy0, dz0;
            T xins, yins, zins;

            // Parameters for the individual contributions
            T contr_m[9], contr_ext[9];

            {
                // Place input coordinates on simplectic lattice.
                T stretchOffset = (x + y + z) * STRETCH_CONSTANT;
                T xs = x + stretchOffset;
                T ys = y + stretchOffset;
                T zs = z + stretchOffset;

                // Floor to get simplectic lattice coordinates of rhombohedron
                // (stretched cube) super-cell.
#ifdef __FAST_MATH__
                T xsbd = std::floor(xs);
                T ysbd = std::floor(ys);
                T zsbd = std::floor(zs);
                xsb = (inttype)xsbd;
                ysb = (inttype)ysbd;
                zsb = (inttype)zsbd;
#else
                xsb = fastFloori(xs);
                ysb = fastFloori(ys);
                zsb = fastFloori(zs);
                T xsbd = (T)xsb;
                T ysbd = (T)ysb;
                T zsbd = (T)zsb;
#endif

                // Skew out to get actual coordinates of rhombohedron origin.
                T squishOffset = (xsbd + ysbd + zsbd) * SQUISH_CONSTANT;
                T xb = xsbd + squishOffset;
                T yb = ysbd + squishOffset;
                T zb = zsbd + squishOffset;

                // Positions relative to origin point.
                dx0 = x - xb;
                dy0 = y - yb;
                dz0 = z - zb;

                // Compute simplectic lattice coordinates relative to rhombohedral origin.
                xins = xs - xsbd;
                yins = ys - ysbd;
                zins = zs - zsbd;
            }

            // These are given values inside the next block, and used afterwards.
            inttype xsv_ext0, ysv_ext0, zsv_ext0;
            inttype xsv_ext1, ysv_ext1, zsv_ext1;
            T dx_ext0, dy_ext0, dz_ext0;
            T dx_ext1, dy_ext1, dz_ext1;

            // Sum together to get a value that determines which cell we are in.
            T inSum = xins + yins + zins;

            if (inSum > (T)1.0 && inSum < (T)2.0) {
                // The point is inside the octahedron (rectified 3-Simplex) inbetween.

                T aScore;
                OSN_BYTE aPoint;
                bool aIsFurtherSide;
                T bScore;
                OSN_BYTE bPoint;
                bool bIsFurtherSide;

                // Decide between point (1,0,0) and (0,1,1) as closest.
                T p1 = xins + yins;
                if (p1 <= (T)1.0) {
                    aScore = (T)1.0 - p1;
                    aPoint = 4;
                    aIsFurtherSide = false;
                }
                else {
                    aScore = p1 - (T)1.0;
                    aPoint = 3;
                    aIsFurtherSide = true;
                }

                // Decide between point (0,1,0) and (1,0,1) as closest.
                T p2 = xins + zins;
                if (p2 <= (T)1.0) {
                    bScore = (T)1.0 - p2;
                    bPoint = 2;
                    bIsFurtherSide = false;
                }
                else {
                    bScore = p2 - (T)1.0;
                    bPoint = 5;
                    bIsFurtherSide = true;
                }

                // The closest out of the two (0,0,1) and (1,1,0) will replace the
                // furthest out of the two decided above if closer.
                T p3 = yins + zins;
                if (p3 > (T)1.0) {
                    T score = p3 - (T)1.0;
                    if (aScore > bScore && bScore < score) {
                        bScore = score;
                        bPoint = 6;
                        bIsFurtherSide = true;
                    }
                    else if (aScore <= bScore && aScore < score) {
                        aScore = score;
                        aPoint = 6;
                        aIsFurtherSide = true;
                    }
                }
                else {
                    T score = (T)1.0 - p3;
                    if (aScore > bScore && bScore < score) {
                        bScore = score;
                        bPoint = 1;
                        bIsFurtherSide = false;
                    }
                    else if (aScore <= bScore && aScore < score) {
                        aScore = score;
                        aPoint = 1;
                        aIsFurtherSide = false;
                    }
                }

                // Where each of the two closest points are determines how the
                // extra two vertices are calculated.
                if (aIsFurtherSide == bIsFurtherSide) {
                    if (aIsFurtherSide) {
                        // Both closest points on (1,1,1) side.

                        // One of the two extra points is (1,1,1)
                        xsv_ext0 = xsb + 1;
                        ysv_ext0 = ysb + 1;
                        zsv_ext0 = zsb + 1;
                        dx_ext0 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                        dy_ext0 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                        dz_ext0 = dz0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);

                        // Other extra point is based on the shared axis.
                        OSN_BYTE c = aPoint & bPoint;
                        if (c & 0x01) {
                            xsv_ext1 = xsb + 2;
                            ysv_ext1 = ysb;
                            zsv_ext1 = zsb;
                            dx_ext1 = dx0 - (T)2.0 - (SQUISH_CONSTANT * (T)2.0);
                            dy_ext1 = dy0 - (SQUISH_CONSTANT * (T)2.0);
                            dz_ext1 = dz0 - (SQUISH_CONSTANT * (T)2.0);
                        }
                        else if (c & 0x02) {
                            xsv_ext1 = xsb;
                            ysv_ext1 = ysb + 2;
                            zsv_ext1 = zsb;
                            dx_ext1 = dx0 - (SQUISH_CONSTANT * (T)2.0);
                            dy_ext1 = dy0 - (T)2.0 - (SQUISH_CONSTANT * (T)2.0);
                            dz_ext1 = dz0 - (SQUISH_CONSTANT * (T)2.0);
                        }
                        else {
                            xsv_ext1 = xsb;
                            ysv_ext1 = ysb;
                            zsv_ext1 = zsb + 2;
                            dx_ext1 = dx0 - (SQUISH_CONSTANT * (T)2.0);
                            dy_ext1 = dy0 - (SQUISH_CONSTANT * (T)2.0);
                            dz_ext1 = dz0 - (T)2.0 - (SQUISH_CONSTANT * (T)2.0);
                        }
                    }
                    else {
                        // Both closest points are on the (0,0,0) side.

                        // One of the two extra points is (0,0,0).
                        xsv_ext0 = xsb;
                        ysv_ext0 = ysb;
                        zsv_ext0 = zsb;
                        dx_ext0 = dx0;
                        dy_ext0 = dy0;
                        dz_ext0 = dz0;

                        // The other extra point is based on the omitted axis.
                        OSN_BYTE c = aPoint | bPoint;
                        if (!(c & 0x01)) {
                            xsv_ext1 = xsb - 1;
                            ysv_ext1 = ysb + 1;
                            zsv_ext1 = zsb + 1;
                            dx_ext1 = dx0 + (T)1.0 - SQUISH_CONSTANT;
                            dy_ext1 = dy0 - (T)1.0 - SQUISH_CONSTANT;
                            dz_ext1 = dz0 - (T)1.0 - SQUISH_CONSTANT;
                        }
                        else if (!(c & 0x02)) {
                            xsv_ext1 = xsb + 1;
                            ysv_ext1 = ysb - 1;
                            zsv_ext1 = zsb + 1;
                            dx_ext1 = dx0 - (T)1.0 - SQUISH_CONSTANT;
                            dy_ext1 = dy0 + (T)1.0 - SQUISH_CONSTANT;
                            dz_ext1 = dz0 - (T)1.0 - SQUISH_CONSTANT;
                        }
                        else {
                            xsv_ext1 = xsb + 1;
                            ysv_ext1 = ysb + 1;
                            zsv_ext1 = zsb - 1;
                            dx_ext1 = dx0 - (T)1.0 - SQUISH_CONSTANT;
                            dy_ext1 = dy0 - (T)1.0 - SQUISH_CONSTANT;
                            dz_ext1 = dz0 + (T)1.0 - SQUISH_CONSTANT;
                        }
                    }
                }
                else {
                    // One point is on the (0,0,0) side, one point is on the (1,1,1) side.

                    OSN_BYTE c1, c2;
                    if (aIsFurtherSide) {
                        c1 = aPoint;
                        c2 = bPoint;
                    }
                    else {
                        c1 = bPoint;
                        c2 = aPoint;
                    }

                    // One contribution is a permutation of (1,1,-1).
                    if (!(c1 & 0x01)) {
                        xsv_ext0 = xsb - 1;
                        ysv_ext0 = ysb + 1;
                        zsv_ext0 = zsb + 1;
                        dx_ext0 = dx0 + (T)1.0 - SQUISH_CONSTANT;
                        dy_ext0 = dy0 - (T)1.0 - SQUISH_CONSTANT;
                        dz_ext0 = dz0 - (T)1.0 - SQUISH_CONSTANT;
                    }
                    else if (!(c1 & 0x02)) {
                        xsv_ext0 = xsb + 1;
                        ysv_ext0 = ysb - 1;
                        zsv_ext0 = zsb + 1;
                        dx_ext0 = dx0 - (T)1.0 - SQUISH_CONSTANT;
                        dy_ext0 = dy0 + (T)1.0 - SQUISH_CONSTANT;
                        dz_ext0 = dz0 - (T)1.0 - SQUISH_CONSTANT;
                    }
                    else {
                        xsv_ext0 = xsb + 1;
                        ysv_ext0 = ysb + 1;
                        zsv_ext0 = zsb - 1;
                        dx_ext0 = dx0 - (T)1.0 - SQUISH_CONSTANT;
                        dy_ext0 = dy0 - (T)1.0 - SQUISH_CONSTANT;
                        dz_ext0 = dz0 + (T)1.0 - SQUISH_CONSTANT;
                    }

                    // One contribution is a permutation of (0,0,2).
                    if (c2 & 0x01) {
                        xsv_ext1 = xsb + 2;
                        ysv_ext1 = ysb;
                        zsv_ext1 = zsb;
                        dx_ext1 = dx0 - (T)2.0 - (SQUISH_CONSTANT * (T)2.0);
                        dy_ext1 = dy0 - (SQUISH_CONSTANT * (T)2.0);
                        dz_ext1 = dz0 - (SQUISH_CONSTANT * (T)2.0);
                    }
                    else if (c2 & 0x02) {
                        xsv_ext1 = xsb;
                        ysv_ext1 = ysb + 2;
                        zsv_ext1 = zsb;
                        dx_ext1 = dx0 - (SQUISH_CONSTANT * (T)2.0);
                        dy_ext1 = dy0 - (T)2.0 - (SQUISH_CONSTANT * (T)2.0);
                        dz_ext1 = dz0 - (SQUISH_CONSTANT * (T)2.0);
                    }
                    else {
                        xsv_ext1 = xsb;
                        ysv_ext1 = ysb;
                        zsv_ext1 = zsb + 2;
                        dx_ext1 = dx0 - (SQUISH_CONSTANT * (T)2.0);
                        dy_ext1 = dy0 - (SQUISH_CONSTANT * (T)2.0);
                        dz_ext1 = dz0 - (T)2.0 - (SQUISH_CONSTANT * (T)2.0);
                    }
                }

                contr_m[0] = contr_ext[0] = 0.0;

                // Contribution (0,0,1).
                T dx1 = dx0 - (T)1.0 - SQUISH_CONSTANT;
                T dy1 = dy0 - SQUISH_CONSTANT;
                T dz1 = dz0 - SQUISH_CONSTANT;
                contr_m[1] = pow2(dx1) + pow2(dy1) + pow2(dz1);
                contr_ext[1] = extrapolate(xsb + 1, ysb, zsb, dx1, dy1, dz1);

                // Contribution (0,1,0).
                T dx2 = dx0 - SQUISH_CONSTANT;
                T dy2 = dy0 - (T)1.0 - SQUISH_CONSTANT;
                T dz2 = dz1;
                contr_m[2] = pow2(dx2) + pow2(dy2) + pow2(dz2);
                contr_ext[2] = extrapolate(xsb, ysb + 1, zsb, dx2, dy2, dz2);

                // Contribution (1,0,0).
                T dx3 = dx2;
                T dy3 = dy1;
                T dz3 = dz0 - (T)1.0 - SQUISH_CONSTANT;
                contr_m[3] = pow2(dx3) + pow2(dy3) + pow2(dz3);
                contr_ext[3] = extrapolate(xsb, ysb, zsb + 1, dx3, dy3, dz3);

                // Contribution (1,1,0).
                T dx4 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                T dy4 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                T dz4 = dz0 - (SQUISH_CONSTANT * (T)2.0);
                contr_m[4] = pow2(dx4) + pow2(dy4) + pow2(dz4);
                contr_ext[4] = extrapolate(xsb + 1, ysb + 1, zsb, dx4, dy4, dz4);

                // Contribution (1,0,1).
                T dx5 = dx4;
                T dy5 = dy0 - (SQUISH_CONSTANT * (T)2.0);
                T dz5 = dz0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                contr_m[5] = pow2(dx5) + pow2(dy5) + pow2(dz5);
                contr_ext[5] = extrapolate(xsb + 1, ysb, zsb + 1, dx5, dy5, dz5);

                // Contribution (0,1,1).
                T dx6 = dx0 - (SQUISH_CONSTANT * (T)2.0);
                T dy6 = dy4;
                T dz6 = dz5;
                contr_m[6] = pow2(dx6) + pow2(dy6) + pow2(dz6);
                contr_ext[6] = extrapolate(xsb, ysb + 1, zsb + 1, dx6, dy6, dz6);

            }
            else if (inSum <= (T)1.0) {
                // The point is inside the tetrahedron (3-Simplex) at (0,0,0)

                // Determine which of (0,0,1), (0,1,0), (1,0,0) are closest.
                OSN_BYTE aPoint = 1;
                T aScore = xins;
                OSN_BYTE bPoint = 2;
                T bScore = yins;
                if (aScore < bScore && zins > aScore) {
                    aScore = zins;
                    aPoint = 4;
                }
                else if (aScore >= bScore && zins > bScore) {
                    bScore = zins;
                    bPoint = 4;
                }

                // Determine the two lattice points not part of the tetrahedron that may contribute.
                // This depends on the closest two tetrahedral vertices, including (0,0,0).
                T wins = (T)1.0 - inSum;
                if (wins > aScore || wins > bScore) {
                    // (0,0,0) is one of the closest two tetrahedral vertices.

                    // The other closest vertex is the closer of a and b.
                    OSN_BYTE c = ((bScore > aScore) ? bPoint : aPoint);

                    if (c != 1) {
                        xsv_ext0 = xsb - 1;
                        xsv_ext1 = xsb;
                        dx_ext0 = dx0 + (T)1.0;
                        dx_ext1 = dx0;
                    }
                    else {
                        xsv_ext0 = xsv_ext1 = xsb + 1;
                        dx_ext0 = dx_ext1 = dx0 - (T)1.0;
                    }

                    if (c != 2) {
                        ysv_ext0 = ysv_ext1 = ysb;
                        dy_ext0 = dy_ext1 = dy0;
                        if (c == 1) {
                            ysv_ext0 -= 1;
                            dy_ext0 += (T)1.0;
                        }
                        else {
                            ysv_ext1 -= 1;
                            dy_ext1 += (T)1.0;
                        }
                    }
                    else {
                        ysv_ext0 = ysv_ext1 = ysb + 1;
                        dy_ext0 = dy_ext1 = dy0 - (T)1.0;
                    }

                    if (c != 4) {
                        zsv_ext0 = zsb;
                        zsv_ext1 = zsb - 1;
                        dz_ext0 = dz0;
                        dz_ext1 = dz0 + (T)1.0;
                    }
                    else {
                        zsv_ext0 = zsv_ext1 = zsb + 1;
                        dz_ext0 = dz_ext1 = dz0 - (T)1.0;
                    }
                }
                else {
                    // (0,0,0) is not one of the closest two tetrahedral vertices.

                    // The two extra vertices are determined by the closest two.
                    OSN_BYTE c = (aPoint | bPoint);

                    if (c & 0x01) {
                        xsv_ext0 = xsv_ext1 = xsb + 1;
                        dx_ext0 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                        dx_ext1 = dx0 - (T)1.0 - SQUISH_CONSTANT;
                    }
                    else {
                        xsv_ext0 = xsb;
                        xsv_ext1 = xsb - 1;
                        dx_ext0 = dx0 - (SQUISH_CONSTANT * (T)2.0);
                        dx_ext1 = dx0 + (T)1.0 - SQUISH_CONSTANT;
                    }

                    if (c & 0x02) {
                        ysv_ext0 = ysv_ext1 = ysb + 1;
                        dy_ext0 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                        dy_ext1 = dy0 - (T)1.0 - SQUISH_CONSTANT;
                    }
                    else {
                        ysv_ext0 = ysb;
                        ysv_ext1 = ysb - 1;
                        dy_ext0 = dy0 - (SQUISH_CONSTANT * (T)2.0);
                        dy_ext1 = dy0 + (T)1.0 - SQUISH_CONSTANT;
                    }

                    if (c & 0x04) {
                        zsv_ext0 = zsv_ext1 = zsb + 1;
                        dz_ext0 = dz0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                        dz_ext1 = dz0 - (T)1.0 - SQUISH_CONSTANT;
                    }
                    else {
                        zsv_ext0 = zsb;
                        zsv_ext1 = zsb - 1;
                        dz_ext0 = dz0 - (SQUISH_CONSTANT * (T)2.0);
                        dz_ext1 = dz0 + (T)1.0 - SQUISH_CONSTANT;
                    }
                }

                // Contribution (0,0,0)
                {
                    contr_m[0] = pow2(dx0) + pow2(dy0) + pow2(dz0);
                    contr_ext[0] = extrapolate(xsb, ysb, zsb, dx0, dy0, dz0);
                }

                // Contribution (0,0,1)
                T dx1 = dx0 - (T)1.0 - SQUISH_CONSTANT;
                T dy1 = dy0 - SQUISH_CONSTANT;
                T dz1 = dz0 - SQUISH_CONSTANT;
                contr_m[1] = pow2(dx1) + pow2(dy1) + pow2(dz1);
                contr_ext[1] = extrapolate(xsb + 1, ysb, zsb, dx1, dy1, dz1);

                // Contribution (0,1,0)
                T dx2 = dx0 - SQUISH_CONSTANT;
                T dy2 = dy0 - (T)1.0 - SQUISH_CONSTANT;
                T dz2 = dz1;
                contr_m[2] = pow2(dx2) + pow2(dy2) + pow2(dz2);
                contr_ext[2] = extrapolate(xsb, ysb + 1, zsb, dx2, dy2, dz2);

                // Contribution (1,0,0)
                T dx3 = dx2;
                T dy3 = dy1;
                T dz3 = dz0 - (T)1.0 - SQUISH_CONSTANT;
                contr_m[3] = pow2(dx3) + pow2(dy3) + pow2(dz3);
                contr_ext[3] = extrapolate(xsb, ysb, zsb + 1, dx3, dy3, dz3);

                contr_m[4] = contr_m[5] = contr_m[6] = 0.0;
                contr_ext[4] = contr_ext[5] = contr_ext[6] = 0.0;

            }
            else {
                // The point is inside the tetrahedron (3-Simplex) at (1,1,1)

                // Determine which two tetrahedral vertices are the closest
                // out of (1,1,0), (1,0,1), and (0,1,1), but not (1,1,1).
                OSN_BYTE aPoint = 6;
                T aScore = xins;
                OSN_BYTE bPoint = 5;
                T bScore = yins;
                if (aScore <= bScore && zins < bScore) {
                    bScore = zins;
                    bPoint = 3;
                }
                else if (aScore > bScore && zins < aScore) {
                    aScore = zins;
                    aPoint = 3;
                }

                // Determine the two lattice points not part of the tetrahedron that may contribute.
                // This depends on the closest two tetrahedral vertices, including (1,1,1).
                T wins = 3.0 - inSum;
                if (wins < aScore || wins < bScore) {
                    // (1,1,1) is one of the closest two tetrahedral vertices.

                    // The other closest vertex is the closest of a and b.
                    OSN_BYTE c = ((bScore < aScore) ? bPoint : aPoint);

                    if (c & 0x01) {
                        xsv_ext0 = xsb + 2;
                        xsv_ext1 = xsb + 1;
                        dx_ext0 = dx0 - (T)2.0 - (SQUISH_CONSTANT * (T)3.0);
                        dx_ext1 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                    }
                    else {
                        xsv_ext0 = xsv_ext1 = xsb;
                        dx_ext0 = dx_ext1 = dx0 - (SQUISH_CONSTANT * (T)3.0);
                    }

                    if (c & 0x02) {
                        ysv_ext0 = ysv_ext1 = ysb + 1;
                        dy_ext0 = dy_ext1 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                        if (c & 0x01) {
                            ysv_ext1 += 1;
                            dy_ext1 -= (T)1.0;
                        }
                        else {
                            ysv_ext0 += 1;
                            dy_ext0 -= (T)1.0;
                        }
                    }
                    else {
                        ysv_ext0 = ysv_ext1 = ysb;
                        dy_ext0 = dy_ext1 = dy0 - (SQUISH_CONSTANT * (T)3.0);
                    }

                    if (c & 0x04) {
                        zsv_ext0 = zsb + 1;
                        zsv_ext1 = zsb + 2;
                        dz_ext0 = dz0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                        dz_ext1 = dz0 - (T)2.0 - (SQUISH_CONSTANT * (T)3.0);
                    }
                    else {
                        zsv_ext0 = zsv_ext1 = zsb;
                        dz_ext0 = dz_ext1 = dz0 - (SQUISH_CONSTANT * (T)3.0);
                    }
                }
                else {
                    // (1,1,1) is not one of the closest two tetrahedral vertices.

                    // The two extra vertices are determined by the closest two.
                    OSN_BYTE c = aPoint & bPoint;

                    if (c & 0x01) {
                        xsv_ext0 = xsb + 1;
                        xsv_ext1 = xsb + 2;
                        dx_ext0 = dx0 - (T)1.0 - SQUISH_CONSTANT;
                        dx_ext1 = dx0 - (T)2.0 - (SQUISH_CONSTANT * (T)2.0);
                    }
                    else {
                        xsv_ext0 = xsv_ext1 = xsb;
                        dx_ext0 = dx0 - SQUISH_CONSTANT;
                        dx_ext1 = dx0 - (SQUISH_CONSTANT * (T)2.0);
                    }

                    if (c & 0x02) {
                        ysv_ext0 = ysb + 1;
                        ysv_ext1 = ysb + 2;
                        dy_ext0 = dy0 - (T)1.0 - SQUISH_CONSTANT;
                        dy_ext1 = dy0 - (T)2.0 - (SQUISH_CONSTANT * (T)2.0);
                    }
                    else {
                        ysv_ext0 = ysv_ext1 = ysb;
                        dy_ext0 = dy0 - SQUISH_CONSTANT;
                        dy_ext1 = dy0 - (SQUISH_CONSTANT * (T)2.0);
                    }

                    if (c & 0x04) {
                        zsv_ext0 = zsb + 1;
                        zsv_ext1 = zsb + 2;
                        dz_ext0 = dz0 - (T)1.0 - SQUISH_CONSTANT;
                        dz_ext1 = dz0 - (T)2.0 - (SQUISH_CONSTANT * (T)2.0);
                    }
                    else {
                        zsv_ext0 = zsv_ext1 = zsb;
                        dz_ext0 = dz0 - SQUISH_CONSTANT;
                        dz_ext1 = dz0 - (SQUISH_CONSTANT * (T)2.0);
                    }
                }

                // Contribution (1,1,0)
                T dx3 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                T dy3 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                T dz3 = dz0 - (SQUISH_CONSTANT * (T)2.0);
                contr_m[3] = pow2(dx3) + pow2(dy3) + pow2(dz3);
                contr_ext[3] = extrapolate(xsb + 1, ysb + 1, zsb, dx3, dy3, dz3);

                // Contribution (1,0,1)
                T dx2 = dx3;
                T dy2 = dy0 - (SQUISH_CONSTANT * (T)2.0);
                T dz2 = dz0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                contr_m[2] = pow2(dx2) + pow2(dy2) + pow2(dz2);
                contr_ext[2] = extrapolate(xsb + 1, ysb, zsb + 1, dx2, dy2, dz2);

                // Contribution (0,1,1)
                {
                    T dx1 = dx0 - (SQUISH_CONSTANT * (T)2.0);
                    T dy1 = dy3;
                    T dz1 = dz2;
                    contr_m[1] = pow2(dx1) + pow2(dy1) + pow2(dz1);
                    contr_ext[1] = extrapolate(xsb, ysb + 1, zsb + 1, dx1, dy1, dz1);
                }

                // Contribution (1,1,1)
                {
                    dx0 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                    dy0 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                    dz0 = dz0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                    contr_m[0] = pow2(dx0) + pow2(dy0) + pow2(dz0);
                    contr_ext[0] = extrapolate(xsb + 1, ysb + 1, zsb + 1, dx0, dy0, dz0);
                }

                contr_m[4] = contr_m[5] = contr_m[6] = 0.0;
                contr_ext[4] = contr_ext[5] = contr_ext[6] = 0.0;

            }

            // First extra vertex.
            contr_m[7] = pow2(dx_ext0) + pow2(dy_ext0) + pow2(dz_ext0);
            contr_ext[7] = extrapolate(xsv_ext0, ysv_ext0, zsv_ext0, dx_ext0, dy_ext0, dz_ext0);

            // Second extra vertex.
            contr_m[8] = pow2(dx_ext1) + pow2(dy_ext1) + pow2(dz_ext1);
            contr_ext[8] = extrapolate(xsv_ext1, ysv_ext1, zsv_ext1, dx_ext1, dy_ext1, dz_ext1);

            T value = 0.0;
            for (int i = 0; i < 9; ++i) {
                value += pow4(std::max((T)2.0 - contr_m[i], (T)0.0)) * contr_ext[i];
            }

            return (value * NORM_CONSTANT);
        }

    };


    // Array of gradient values for 3D. They approximate the directions to the
    // vertices of a rhombicuboctahedron from its center, skewed so that the
    // triangular and square facets can be inscribed in circles of the same radius.
    // New gradient set 2014-10-06.
    const int Noise<3>::gradients[] = {
      -11, 4, 4,  -4, 11, 4,  -4, 4, 11,   11, 4, 4,   4, 11, 4,   4, 4, 11,
      -11,-4, 4,  -4,-11, 4,  -4,-4, 11,   11,-4, 4,   4,-11, 4,   4,-4, 11,
      -11, 4,-4,  -4, 11,-4,  -4, 4,-11,   11, 4,-4,   4, 11,-4,   4, 4,-11,
      -11,-4,-4,  -4,-11,-4,  -4,-4,-11,   11,-4,-4,   4,-11,-4,   4,-4,-11
    };


    // 4D Implementation of the OpenSimplexNoise generator.
    template <>
    class Noise <4> : public NoiseBase {
    private:

        // Array of gradient values for 4D. Values are defined below the class definition.
        static const int gradients[256];

        template <typename T>
        inline T extrapolate(inttype xsb, inttype ysb, inttype zsb, inttype wsb, T dx, T dy, T dz, T dw) const {
            unsigned int index = perm[(perm[(perm[(perm[xsb & 0xFF] + ysb) & 0xFF] + zsb) & 0xFF] + wsb) & 0xFF] & 0xFC;
            return gradients[index] * dx +
                gradients[index + 1] * dy +
                gradients[index + 2] * dz +
                gradients[index + 3] * dw;
        }

    public:

#ifdef OSN_USE_CSTDINT
        Noise(int64_t seed = 0LL) : NoiseBase(seed) {}
#else
        Noise(long seed = 0L) : NoiseBase(seed) {}
#endif
        Noise(const int* p) : NoiseBase(p) {}


        template <typename T>
        T eval(T x, T y, T z, T w) const {

#ifdef OSN_USE_STATIC_ASSERT
            static_assert(std::is_floating_point<T>::value, "OpenSimplexNoise can only be used with floating-point types");
#endif

            static const T STRETCH_CONSTANT = (T)((1.0 / std::sqrt(4.0 + 1.0) - 1.0) * 0.25);
            static const T SQUISH_CONSTANT = (T)((std::sqrt(4.0 + 1.0) - 1.0) * 0.25);
            static const T NORM_CONSTANT = (T)(1.0 / 30.0);

            T dx0, dy0, dz0, dw0;
            inttype xsb, ysb, zsb, wsb;
            T xins, yins, zins, wins;

            {
                // Place input coordinates on simplectic honeycomb.
                T stretchOffset = (x + y + z + w) * STRETCH_CONSTANT;
                T xs = x + stretchOffset;
                T ys = y + stretchOffset;
                T zs = z + stretchOffset;
                T ws = w + stretchOffset;

                // Floor to get simplectic honeycomb coordinates of rhombo-hypercube origin.
#ifdef __FAST_MATH__
                T xsbd = std::floor(xs);
                T ysbd = std::floor(ys);
                T zsbd = std::floor(zs);
                T wsbd = std::floor(ws);
                xsb = (inttype)xsbd;
                ysb = (inttype)ysbd;
                zsb = (inttype)zsbd;
                wsb = (inttype)wsbd;
#else
                xsb = fastFloori(xs);
                ysb = fastFloori(ys);
                zsb = fastFloori(zs);
                wsb = fastFloori(ws);
                T xsbd = (T)xsb;
                T ysbd = (T)ysb;
                T zsbd = (T)zsb;
                T wsbd = (T)wsb;
#endif

                // Skew out to get actual coordinates of stretched rhombo-hypercube origin.
                T squishOffset = (xsbd + ysbd + zsbd + wsbd) * SQUISH_CONSTANT;
                T xb = xsbd + squishOffset;
                T yb = ysbd + squishOffset;
                T zb = zsbd + squishOffset;
                T wb = wsbd + squishOffset;

                // Positions relative to origin point.
                dx0 = x - xb;
                dy0 = y - yb;
                dz0 = z - zb;
                dw0 = w - wb;

                // Compute simplectic honeycomb coordinates relative to rhombo-hypercube origin.
                xins = xs - xsbd;
                yins = ys - ysbd;
                zins = zs - zsbd;
                wins = ws - wsbd;
            }

            // These are given values inside the next block, and used afterwards.
            inttype xsv_ext0, ysv_ext0, zsv_ext0, wsv_ext0;
            inttype xsv_ext1, ysv_ext1, zsv_ext1, wsv_ext1;
            inttype xsv_ext2, ysv_ext2, zsv_ext2, wsv_ext2;
            T dx_ext0, dy_ext0, dz_ext0, dw_ext0;
            T dx_ext1, dy_ext1, dz_ext1, dw_ext1;
            T dx_ext2, dy_ext2, dz_ext2, dw_ext2;

            T value = 0.0;

            // Sum together to get a value that determines which cell we are in.
            T inSum = xins + yins + zins + wins;

            if (inSum <= (T)1.0) {
                // Inside a pentachoron (4-Simplex) at (0,0,0,0)

                // Determine which two of (0,0,0,1), (0,0,1,0), (0,1,0,0) and (1,0,0,0) are closest.
                OSN_BYTE aPoint = 0x01, bPoint = 0x02;
                T aScore = xins, bScore = yins;
                if (aScore >= bScore && zins > bScore) {
                    bPoint = 0x04;
                    bScore = zins;
                }
                else if (aScore < bScore && zins > aScore) {
                    aPoint = 0x04;
                    aScore = zins;
                }
                if (aScore >= bScore && wins > bScore) {
                    bPoint = 0x08;
                    bScore = wins;
                }
                else if (aScore < bScore && wins > aScore) {
                    aPoint = 0x08;
                    aScore = wins;
                }

                // Determine the three lattice points not part of the pentachoron
                // that may contribute.
                // This depends on the closest two pentachoron vertices, including (0,0,0,0).
                T uins = (T)1.0 - inSum;
                if (uins > aScore || uins > bScore) {
                    // (0,0,0,0) is one of the closest two pentachoron vertices.

                    // The other closest vertex is the closest out of A and B.
                    OSN_BYTE c = (bScore > aScore ? bPoint : aPoint);

                    if (c != 0x01) {
                        xsv_ext0 = xsb - 1;
                        xsv_ext1 = xsv_ext2 = xsb;
                        dx_ext0 = dx0 + (T)1.0;
                        dx_ext1 = dx_ext2 = dx0;
                    }
                    else {
                        xsv_ext0 = xsv_ext1 = xsv_ext2 = xsb + 1;
                        dx_ext0 = dx_ext1 = dx_ext2 = dx0 - (T)1.0;
                    }

                    if (c != 0x02) {
                        ysv_ext0 = ysv_ext1 = ysv_ext2 = ysb;
                        dy_ext0 = dy_ext1 = dy_ext2 = dy0;
                        if (c != 0x01) {
                            ysv_ext1 -= 1;
                            dy_ext1 += (T)1.0;
                        }
                        else {
                            ysv_ext0 -= 1;
                            dy_ext0 += (T)1.0;
                        }
                    }
                    else {
                        ysv_ext0 = ysv_ext1 = ysv_ext2 = ysb + 1;
                        dy_ext0 = dy_ext1 = dy_ext2 = dy0 - (T)1.0;
                    }

                    if (c != 0x04) {
                        zsv_ext0 = zsv_ext1 = zsv_ext2 = zsb;
                        dz_ext0 = dz_ext1 = dz_ext2 = dz0;
                        if (c & 0x03) {
                            zsv_ext1 -= 1;
                            dz_ext1 += (T)1.0;
                        }
                        else {
                            zsv_ext2 -= 1;
                            dz_ext2 += (T)1.0;
                        }
                    }
                    else {
                        zsv_ext0 = zsv_ext1 = zsv_ext2 = zsb + 1;
                        dz_ext0 = dz_ext1 = dz_ext2 = dz0 - (T)1.0;
                    }

                    if (c != 0x08) {
                        wsv_ext0 = wsv_ext1 = wsb;
                        wsv_ext2 = wsb - 1;
                        dw_ext0 = dw_ext1 = dw0;
                        dw_ext2 = dw0 + (T)1.0;
                    }
                    else {
                        wsv_ext0 = wsv_ext1 = wsv_ext2 = wsb + 1;
                        dw_ext0 = dw_ext1 = dw_ext2 = dw0 - (T)1.0;
                    }
                }
                else {
                    // (0,0,0,0) is not one of the closest two pentachoron vertices.

                    // The three extra vertices are determined by the closest two.
                    OSN_BYTE c = (aPoint | bPoint);

                    if (!(c & 0x01)) {
                        xsv_ext0 = xsv_ext2 = xsb;
                        xsv_ext1 = xsb - 1;
                        dx_ext0 = dx0 - (SQUISH_CONSTANT * (T)2.0);
                        dx_ext1 = dx0 + (T)1.0 - SQUISH_CONSTANT;
                        dx_ext2 = dx0 - SQUISH_CONSTANT;
                    }
                    else {
                        xsv_ext0 = xsv_ext1 = xsv_ext2 = xsb + 1;
                        dx_ext0 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                        dx_ext1 = dx_ext2 = dx0 - (T)1.0 - SQUISH_CONSTANT;
                    }

                    if (!(c & 0x02)) {
                        ysv_ext0 = ysv_ext1 = ysv_ext2 = ysb;
                        dy_ext0 = dy0 - (SQUISH_CONSTANT * (T)2.0);
                        dy_ext1 = dy_ext2 = dy0 - SQUISH_CONSTANT;
                        if (c & 0x01) {
                            ysv_ext1 -= 1;
                            dy_ext1 += (T)1.0;
                        }
                        else {
                            ysv_ext2 -= 1;
                            dy_ext2 += (T)1.0;
                        }
                    }
                    else {
                        ysv_ext0 = ysv_ext1 = ysv_ext2 = ysb + 1;
                        dy_ext0 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                        dy_ext1 = dy_ext2 = dy0 - (T)1.0 - SQUISH_CONSTANT;
                    }

                    if (!(c & 0x04)) {
                        zsv_ext0 = zsv_ext1 = zsv_ext2 = zsb;
                        dz_ext0 = dz0 - (SQUISH_CONSTANT * (T)2.0);
                        dz_ext1 = dz_ext2 = dz0 - SQUISH_CONSTANT;
                        if (c & 0x03) {
                            zsv_ext1 -= 1;
                            dz_ext1 += (T)1.0;
                        }
                        else {
                            zsv_ext2 -= 1;
                            dz_ext2 += (T)1.0;
                        }
                    }
                    else {
                        zsv_ext0 = zsv_ext1 = zsv_ext2 = zsb + 1;
                        dz_ext0 = dz0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                        dz_ext1 = dz_ext2 = dz0 - (T)1.0 - SQUISH_CONSTANT;
                    }

                    if (!(c & 0x08)) {
                        wsv_ext0 = wsv_ext1 = wsb;
                        wsv_ext2 = wsb - 1;
                        dw_ext0 = dw0 - (SQUISH_CONSTANT * (T)2.0);
                        dw_ext1 = dw0 - SQUISH_CONSTANT;
                        dw_ext2 = dw0 + (T)1.0 - SQUISH_CONSTANT;
                    }
                    else {
                        wsv_ext0 = wsv_ext1 = wsv_ext2 = wsb + 1;
                        dw_ext0 = dw0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                        dw_ext1 = dw_ext2 = dw0 - (T)1.0 - SQUISH_CONSTANT;
                    }
                }

                // Contribution (0,0,0,0).
                {
                    T attn = pow2(dx0) + pow2(dy0) + pow2(dz0) + pow2(dw0);
                    value = pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb, ysb, zsb, wsb, dx0, dy0, dz0, dw0);
                }

                // Contribution (1,0,0,0).
                T dx1 = dx0 - (T)1.0 - SQUISH_CONSTANT;
                T dy1 = dy0 - SQUISH_CONSTANT;
                T dz1 = dz0 - SQUISH_CONSTANT;
                T dw1 = dw0 - SQUISH_CONSTANT;
                {
                    T attn = pow2(dx1) + pow2(dy1) + pow2(dz1) + pow2(dw1);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb + 1, ysb, zsb, wsb, dx1, dy1, dz1, dw1);
                }

                // Contribution (0,1,0,0).
                T dx2 = dx0 - SQUISH_CONSTANT;
                T dy2 = dy0 - (T)1.0 - SQUISH_CONSTANT;
                T dz2 = dz1;
                T dw2 = dw1;
                {
                    T attn = pow2(dx2) + pow2(dy2) + pow2(dz2) + pow2(dw2);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb, ysb + 1, zsb, wsb, dx2, dy2, dz2, dw2);
                }

                // Contribution (0,0,1,0).
                {
                    T dx3 = dx2;
                    T dy3 = dy1;
                    T dz3 = dz0 - (T)1.0 - SQUISH_CONSTANT;
                    T dw3 = dw1;
                    T attn = pow2(dx3) + pow2(dy3) + pow2(dz3) + pow2(dw3);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb, ysb, zsb + 1, wsb, dx3, dy3, dz3, dw3);
                }

                // Contribution (0,0,0,1).
                {
                    T dx4 = dx2;
                    T dy4 = dy1;
                    T dz4 = dz1;
                    T dw4 = dw0 - (T)1.0 - SQUISH_CONSTANT;
                    T attn = pow2(dx4) + pow2(dy4) + pow2(dz4) + pow2(dw4);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb, ysb, zsb, wsb + 1, dx4, dy4, dz4, dw4);
                }

            }
            else if (inSum >= 3.0) {
                // Inside the pentachoron (4-simplex) at (1,1,1,1).

                // Determine which two of (1,1,1,0), (1,1,0,1), (1,0,1,1), (0,1,1,1) are closest.
                OSN_BYTE aPoint = 0x0E;
                T aScore = xins;
                OSN_BYTE bPoint = 0x0D;
                T bScore = yins;
                if (aScore <= bScore && zins < bScore) {
                    bPoint = 0x0B;
                    bScore = zins;
                }
                else if (aScore > bScore && zins < aScore) {
                    aPoint = 0x0B;
                    aScore = zins;
                }
                if (aScore <= bScore && wins < bScore) {
                    bPoint = 0x07;
                    bScore = wins;
                }
                else if (aScore > bScore && wins < aScore) {
                    aPoint = 0x07;
                    aScore = wins;
                }

                // Determine the three lattice points not part of the pentachoron that may contribute.
                // This depends on the closest two pentachoron vertices, including (0,0,0,0).
                T uins = 4.0 - inSum;
                if (uins < aScore || uins < bScore) {
                    // (1,1,1,1) is one of the closest two pentachoron vertices.

                    // The other closest vertex is the closest out of A and B.
                    OSN_BYTE c = (bScore < aScore ? bPoint : aPoint);
                    if (c & 0x01) {
                        xsv_ext0 = xsb + 2;
                        xsv_ext1 = xsv_ext2 = xsb + 1;
                        dx_ext0 = dx0 - (T)2.0 - (SQUISH_CONSTANT * 4);
                        dx_ext1 = dx_ext2 = dx0 - (T)1.0 - (SQUISH_CONSTANT * 4);
                    }
                    else {
                        xsv_ext0 = xsv_ext1 = xsv_ext2 = xsb;
                        dx_ext0 = dx_ext1 = dx_ext2 = dx0 - (SQUISH_CONSTANT * 4);
                    }

                    if (c & 0x02) {
                        ysv_ext0 = ysv_ext1 = ysv_ext2 = ysb + 1;
                        dy_ext0 = dy_ext1 = dy_ext2 = dy0 - (T)1.0 - (SQUISH_CONSTANT * 4);
                        if (c & 0x01) {
                            ysv_ext1 += 1;
                            dy_ext1 -= (T)1.0;
                        }
                        else {
                            ysv_ext0 += 1;
                            dy_ext0 -= (T)1.0;
                        }
                    }
                    else {
                        ysv_ext0 = ysv_ext1 = ysv_ext2 = ysb;
                        dy_ext0 = dy_ext1 = dy_ext2 = dy0 - (SQUISH_CONSTANT * 4);
                    }

                    if (c & 0x04) {
                        zsv_ext0 = zsv_ext1 = zsv_ext2 = zsb + 1;
                        dz_ext0 = dz_ext1 = dz_ext2 = dz0 - (T)1.0 - (SQUISH_CONSTANT * 4);
                        if ((c & 0x03) != 0x03) {
                            if (!(c & 0x03)) {
                                zsv_ext0 += 1;
                                dz_ext0 -= (T)1.0;
                            }
                            else {
                                zsv_ext1 += 1;
                                dz_ext1 -= (T)1.0;
                            }
                        }
                        else {
                            zsv_ext2 += 1;
                            dz_ext2 -= (T)1.0;
                        }
                    }
                    else {
                        zsv_ext0 = zsv_ext1 = zsv_ext2 = zsb;
                        dz_ext0 = dz_ext1 = dz_ext2 = dz0 - (SQUISH_CONSTANT * 4);
                    }

                    if (c & 0x08) {
                        wsv_ext0 = wsv_ext1 = wsb + 1;
                        wsv_ext2 = wsb + 2;
                        dw_ext0 = dw_ext1 = dw0 - (T)1.0 - (SQUISH_CONSTANT * 4);
                        dw_ext2 = dw0 - (T)2.0 - (SQUISH_CONSTANT * 4);
                    }
                    else {
                        wsv_ext0 = wsv_ext1 = wsv_ext2 = wsb;
                        dw_ext0 = dw_ext1 = dw_ext2 = dw0 - (SQUISH_CONSTANT * 4);
                    }
                }
                else {
                    // (1,1,1,1) is not one of the closest two pentachoron vertices.

                    OSN_BYTE c = aPoint & bPoint;
                    if (c & 0x01) {
                        xsv_ext0 = xsv_ext2 = xsb + 1;
                        xsv_ext1 = xsb + 2;
                        dx_ext0 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                        dx_ext1 = dx0 - (T)2.0 - (SQUISH_CONSTANT * (T)3.0);
                        dx_ext2 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                    }
                    else {
                        xsv_ext0 = xsv_ext1 = xsv_ext2 = xsb;
                        dx_ext0 = dx0 - (SQUISH_CONSTANT * (T)2.0);
                        dx_ext1 = dx_ext2 = dx0 - (SQUISH_CONSTANT * (T)3.0);
                    }

                    if (c & 0x02) {
                        ysv_ext0 = ysv_ext1 = ysv_ext2 = ysb + 1;
                        dy_ext0 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                        dy_ext1 = dy_ext2 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                        if (c & 0x01) {
                            ysv_ext2 += 1;
                            dy_ext2 -= (T)1.0;
                        }
                        else {
                            ysv_ext1 += 1;
                            dy_ext1 -= (T)1.0;
                        }
                    }
                    else {
                        ysv_ext0 = ysv_ext1 = ysv_ext2 = ysb;
                        dy_ext0 = dy0 - (SQUISH_CONSTANT * (T)2.0);
                        dy_ext1 = dy_ext2 = dy0 - (SQUISH_CONSTANT * (T)3.0);
                    }

                    if (c & 0x04) {
                        zsv_ext0 = zsv_ext1 = zsv_ext2 = zsb + 1;
                        dz_ext0 = dz0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                        dz_ext1 = dz_ext2 = dz0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                        if (c & 0x03) {
                            zsv_ext2 += 1;
                            dz_ext2 -= (T)1.0;
                        }
                        else {
                            zsv_ext1 += 1;
                            dz_ext1 -= (T)1.0;
                        }
                    }
                    else {
                        zsv_ext0 = zsv_ext1 = zsv_ext2 = zsb;
                        dz_ext0 = dz0 - (SQUISH_CONSTANT * (T)2.0);
                        dz_ext1 = dz_ext2 = dz0 - (SQUISH_CONSTANT * (T)3.0);
                    }

                    if (c & 0x08) {
                        wsv_ext0 = wsv_ext1 = wsb + 1;
                        wsv_ext2 = wsb + 2;
                        dw_ext0 = dw0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                        dw_ext1 = dw0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                        dw_ext2 = dw0 - (T)2.0 - (SQUISH_CONSTANT * (T)3.0);
                    }
                    else {
                        wsv_ext0 = wsv_ext1 = wsv_ext2 = wsb;
                        dw_ext0 = dw0 - (SQUISH_CONSTANT * (T)2.0);
                        dw_ext1 = dw_ext2 = dw0 - (SQUISH_CONSTANT * (T)3.0);
                    }
                }

                // Contribution (1,1,1,0).
                T dx4 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                T dy4 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                T dz4 = dz0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                T dw4 = dw0 - (SQUISH_CONSTANT * (T)3.0);
                {
                    T attn = pow2(dx4) + pow2(dy4) + pow2(dz4) + pow2(dw4);
                    value = pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb + 1, ysb + 1, zsb + 1, wsb, dx4, dy4, dz4, dw4);
                }

                // Contribution (1,1,0,1).
                T dx3 = dx4;
                T dy3 = dy4;
                T dz3 = dz0 - (SQUISH_CONSTANT * (T)3.0);
                T dw3 = dw0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                {
                    T attn = pow2(dx3) + pow2(dy3) + pow2(dz3) + pow2(dw3);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb + 1, ysb + 1, zsb, wsb + 1, dx3, dy3, dz3, dw3);
                }

                // Contribution (1,0,1,1).
                T dx2 = dx4;
                T dy2 = dy0 - (SQUISH_CONSTANT * (T)3.0);
                T dz2 = dz4;
                T dw2 = dw3;
                {
                    T attn = pow2(dx2) + pow2(dy2) + pow2(dz2) + pow2(dw2);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb + 1, ysb, zsb + 1, wsb + 1, dx2, dy2, dz2, dw2);
                }

                // Contribution (0,1,1,1).
                {
                    T dx1 = dx0 - (SQUISH_CONSTANT * (T)3.0);
                    T dy1 = dy4;
                    T dz1 = dz4;
                    T dw1 = dw3;
                    T attn = pow2(dx1) + pow2(dy1) + pow2(dz1) + pow2(dw1);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb, ysb + 1, zsb + 1, wsb + 1, dx1, dy1, dz1, dw1);
                }

                // Contribution (1,1,1,1).
                {
                    dx0 = dx0 - (T)1.0 - (SQUISH_CONSTANT * 4);
                    dy0 = dy0 - (T)1.0 - (SQUISH_CONSTANT * 4);
                    dz0 = dz0 - (T)1.0 - (SQUISH_CONSTANT * 4);
                    dw0 = dw0 - (T)1.0 - (SQUISH_CONSTANT * 4);
                    T attn = pow2(dx0) + pow2(dy0) + pow2(dz0) + pow2(dw0);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb + 1, ysb + 1, zsb + 1, wsb + 1, dx0, dy0, dz0, dw0);
                }

            }
            else if (inSum <= (T)2.0) {
                // Inside the first dispentachoron (rectified 4-simplex).

                T aScore;
                OSN_BYTE aPoint;
                bool aIsBiggerSide = true;
                T bScore;
                OSN_BYTE bPoint;
                bool bIsBiggerSide = true;

                // Decide between (1,1,0,0) and (0,0,1,1).
                if (xins + yins > zins + wins) {
                    aPoint = 0x03;
                    aScore = xins + yins;
                }
                else {
                    aPoint = 0x0C;
                    aScore = zins + wins;
                }

                // Decide between (1,0,1,0) and (0,1,0,1).
                if (xins + zins > yins + wins) {
                    bPoint = 0x05;
                    bScore = xins + zins;
                }
                else {
                    bPoint = 0x0A;
                    bScore = yins + wins;
                }

                // Closer of (1,0,0,1) and (0,1,1,0) will replace the further of A and B, if closer.
                if (xins + wins > yins + zins) {
                    T score = xins + wins;
                    if (aScore >= bScore && score > bScore) {
                        bPoint = 0x09;
                        bScore = score;
                    }
                    else if (aScore < bScore && score > aScore) {
                        aPoint = 0x09;
                        aScore = score;
                    }
                }
                else {
                    T score = yins + zins;
                    if (aScore >= bScore && score > bScore) {
                        bPoint = 0x06;
                        bScore = score;
                    }
                    else if (aScore < bScore && score > aScore) {
                        aPoint = 0x06;
                        aScore = score;
                    }
                }

                // Decide if (1,0,0,0) is closer.
                T p1 = (T)2.0 - inSum + xins;
                if (aScore >= bScore && p1 > bScore) {
                    bPoint = 0x01;
                    bScore = p1;
                    bIsBiggerSide = false;
                }
                else if (aScore < bScore && p1 > aScore) {
                    aPoint = 0x01;
                    aScore = p1;
                    aIsBiggerSide = false;
                }

                // Decide if (0,1,0,0) is closer.
                T p2 = (T)2.0 - inSum + yins;
                if (aScore >= bScore && p2 > bScore) {
                    bPoint = 0x02;
                    bScore = p2;
                    bIsBiggerSide = false;
                }
                else if (aScore < bScore && p2 > aScore) {
                    aPoint = 0x02;
                    aScore = p2;
                    aIsBiggerSide = false;
                }

                // Decide if (0,0,1,0) is closer.
                T p3 = (T)2.0 - inSum + zins;
                if (aScore >= bScore && p3 > bScore) {
                    bPoint = 0x04;
                    bScore = p3;
                    bIsBiggerSide = false;
                }
                else if (aScore < bScore && p3 > aScore) {
                    aPoint = 0x04;
                    aScore = p3;
                    aIsBiggerSide = false;
                }

                // Decide if (0,0,0,1) is closer.
                T p4 = (T)2.0 - inSum + wins;
                if (aScore >= bScore && p4 > bScore) {
                    bPoint = 0x08;
                    bScore = p4;
                    bIsBiggerSide = false;
                }
                else if (aScore < bScore && p4 > aScore) {
                    aPoint = 0x08;
                    aScore = p4;
                    aIsBiggerSide = false;
                }

                // Where each of the two closest points are determines how the extra three vertices are calculated.
                if (aIsBiggerSide == bIsBiggerSide) {
                    if (aIsBiggerSide) {
                        // Both closest points are on the bigger side.

                        OSN_BYTE c1 = aPoint | bPoint;
                        OSN_BYTE c2 = aPoint & bPoint;
                        if (!(c1 & 0x01)) {
                            xsv_ext0 = xsb;
                            xsv_ext1 = xsb - 1;
                            dx_ext0 = dx0 - (SQUISH_CONSTANT * (T)3.0);
                            dx_ext1 = dx0 + (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                        }
                        else {
                            xsv_ext0 = xsv_ext1 = xsb + 1;
                            dx_ext0 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                            dx_ext1 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                        }

                        if (!(c1 & 0x02)) {
                            ysv_ext0 = ysb;
                            ysv_ext1 = ysb - 1;
                            dy_ext0 = dy0 - (SQUISH_CONSTANT * (T)3.0);
                            dy_ext1 = dy0 + (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                        }
                        else {
                            ysv_ext0 = ysv_ext1 = ysb + 1;
                            dy_ext0 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                            dy_ext1 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                        }

                        if (!(c1 & 0x04)) {
                            zsv_ext0 = zsb;
                            zsv_ext1 = zsb - 1;
                            dz_ext0 = dz0 - (SQUISH_CONSTANT * (T)3.0);
                            dz_ext1 = dz0 + (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                        }
                        else {
                            zsv_ext0 = zsv_ext1 = zsb + 1;
                            dz_ext0 = dz0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                            dz_ext1 = dz0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                        }

                        if (!(c1 & 0x08)) {
                            wsv_ext0 = wsb;
                            wsv_ext1 = wsb - 1;
                            dw_ext0 = dw0 - (SQUISH_CONSTANT * (T)3.0);
                            dw_ext1 = dw0 + (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                        }
                        else {
                            wsv_ext0 = wsv_ext1 = wsb + 1;
                            dw_ext0 = dw0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                            dw_ext1 = dw0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                        }

                        // One combination is a permutation of (0,0,0,2) based on c2.
                        xsv_ext2 = xsb;
                        ysv_ext2 = ysb;
                        zsv_ext2 = zsb;
                        wsv_ext2 = wsb;
                        dx_ext2 = dx0 - (SQUISH_CONSTANT * (T)2.0);
                        dy_ext2 = dy0 - (SQUISH_CONSTANT * (T)2.0);
                        dz_ext2 = dz0 - (SQUISH_CONSTANT * (T)2.0);
                        dw_ext2 = dw0 - (SQUISH_CONSTANT * (T)2.0);
                        if (c2 & 0x01) {
                            xsv_ext2 += 2;
                            dx_ext2 -= (T)2.0;
                        }
                        else if (c2 & 0x02) {
                            ysv_ext2 += 2;
                            dy_ext2 -= (T)2.0;
                        }
                        else if (c2 & 0x04) {
                            zsv_ext2 += 2;
                            dz_ext2 -= (T)2.0;
                        }
                        else {
                            wsv_ext2 += 2;
                            dw_ext2 -= (T)2.0;
                        }
                    }
                    else {
                        // Both closest points are on the smaller side.

                        // One of the two extra points is (0,0,0,0).
                        xsv_ext2 = xsb;
                        ysv_ext2 = ysb;
                        zsv_ext2 = zsb;
                        wsv_ext2 = wsb;
                        dx_ext2 = dx0;
                        dy_ext2 = dy0;
                        dz_ext2 = dz0;
                        dw_ext2 = dw0;

                        // The other two points are based on the omitted axes.
                        OSN_BYTE c = aPoint | bPoint;
                        if (!(c & 0x01)) {
                            xsv_ext0 = xsb - 1;
                            xsv_ext1 = xsb;
                            dx_ext0 = dx0 + (T)1.0 - SQUISH_CONSTANT;
                            dx_ext1 = dx0 - SQUISH_CONSTANT;
                        }
                        else {
                            xsv_ext0 = xsv_ext1 = xsb + 1;
                            dx_ext0 = dx_ext1 = dx0 - (T)1.0 - SQUISH_CONSTANT;
                        }

                        if (!(c & 0x02)) {
                            ysv_ext0 = ysv_ext1 = ysb;
                            dy_ext0 = dy_ext1 = dy0 - SQUISH_CONSTANT;
                            if (c & 0x01) {
                                ysv_ext0 -= 1;
                                dy_ext0 += (T)1.0;
                            }
                            else {
                                ysv_ext1 -= 1;
                                dy_ext1 += (T)1.0;
                            }
                        }
                        else {
                            ysv_ext0 = ysv_ext1 = ysb + 1;
                            dy_ext0 = dy_ext1 = dy0 - (T)1.0 - SQUISH_CONSTANT;
                        }

                        if (!(c & 0x04)) {
                            zsv_ext0 = zsv_ext1 = zsb;
                            dz_ext0 = dz_ext1 = dz0 - SQUISH_CONSTANT;
                            if ((c & 0x03) == 0x03)
                            {
                                zsv_ext0 -= 1;
                                dz_ext0 += (T)1.0;
                            }
                            else {
                                zsv_ext1 -= 1;
                                dz_ext1 += (T)1.0;
                            }
                        }
                        else {
                            zsv_ext0 = zsv_ext1 = zsb + 1;
                            dz_ext0 = dz_ext1 = dz0 - (T)1.0 - SQUISH_CONSTANT;
                        }

                        if (!(c & 0x08)) {
                            wsv_ext0 = wsb;
                            wsv_ext1 = wsb - 1;
                            dw_ext0 = dw0 - SQUISH_CONSTANT;
                            dw_ext1 = dw0 + (T)1.0 - SQUISH_CONSTANT;
                        }
                        else {
                            wsv_ext0 = wsv_ext1 = wsb + 1;
                            dw_ext0 = dw_ext1 = dw0 - (T)1.0 - SQUISH_CONSTANT;
                        }
                    }
                }
                else {
                    // One point on each side.

                    OSN_BYTE c1, c2;
                    if (aIsBiggerSide) {
                        c1 = aPoint;
                        c2 = bPoint;
                    }
                    else {
                        c1 = bPoint;
                        c2 = aPoint;
                    }

                    // Two contributions are the bigger-sided point with each 0 replaced with -1.
                    if (!(c1 & 0x01)) {
                        xsv_ext0 = xsb - 1;
                        xsv_ext1 = xsb;
                        dx_ext0 = dx0 + (T)1.0 - SQUISH_CONSTANT;
                        dx_ext1 = dx0 - SQUISH_CONSTANT;
                    }
                    else {
                        xsv_ext0 = xsv_ext1 = xsb + 1;
                        dx_ext0 = dx_ext1 = dx0 - (T)1.0 - SQUISH_CONSTANT;
                    }

                    if (!(c1 & 0x02)) {
                        ysv_ext0 = ysv_ext1 = ysb;
                        dy_ext0 = dy_ext1 = dy0 - SQUISH_CONSTANT;
                        if ((c1 & 0x01) == 0x01) {
                            ysv_ext0 -= 1;
                            dy_ext0 += (T)1.0;
                        }
                        else {
                            ysv_ext1 -= 1;
                            dy_ext1 += (T)1.0;
                        }
                    }
                    else {
                        ysv_ext0 = ysv_ext1 = ysb + 1;
                        dy_ext0 = dy_ext1 = dy0 - (T)1.0 - SQUISH_CONSTANT;
                    }

                    if (!(c1 & 0x04)) {
                        zsv_ext0 = zsv_ext1 = zsb;
                        dz_ext0 = dz_ext1 = dz0 - SQUISH_CONSTANT;
                        if ((c1 & 0x03) == 0x03) {
                            zsv_ext0 -= 1;
                            dz_ext0 += (T)1.0;
                        }
                        else {
                            zsv_ext1 -= 1;
                            dz_ext1 += (T)1.0;
                        }
                    }
                    else {
                        zsv_ext0 = zsv_ext1 = zsb + 1;
                        dz_ext0 = dz_ext1 = dz0 - (T)1.0 - SQUISH_CONSTANT;
                    }

                    if (!(c1 & 0x08)) {
                        wsv_ext0 = wsb;
                        wsv_ext1 = wsb - 1;
                        dw_ext0 = dw0 - SQUISH_CONSTANT;
                        dw_ext1 = dw0 + (T)1.0 - SQUISH_CONSTANT;
                    }
                    else {
                        wsv_ext0 = wsv_ext1 = wsb + 1;
                        dw_ext0 = dw_ext1 = dw0 - (T)1.0 - SQUISH_CONSTANT;
                    }

                    // One contribution is a permutation of (0,0,0,2) based on the smaller-sided point.
                    xsv_ext2 = xsb;
                    ysv_ext2 = ysb;
                    zsv_ext2 = zsb;
                    wsv_ext2 = wsb;
                    dx_ext2 = dx0 - (SQUISH_CONSTANT * (T)2.0);
                    dy_ext2 = dy0 - (SQUISH_CONSTANT * (T)2.0);
                    dz_ext2 = dz0 - (SQUISH_CONSTANT * (T)2.0);
                    dw_ext2 = dw0 - (SQUISH_CONSTANT * (T)2.0);
                    if ((c2 & 0x01) != 0) {
                        xsv_ext2 += 2;
                        dx_ext2 -= (T)2.0;
                    }
                    else if ((c2 & 0x02) != 0) {
                        ysv_ext2 += 2;
                        dy_ext2 -= (T)2.0;
                    }
                    else if ((c2 & 0x04) != 0) {
                        zsv_ext2 += 2;
                        dz_ext2 -= (T)2.0;
                    }
                    else {
                        wsv_ext2 += 2;
                        dw_ext2 -= (T)2.0;
                    }
                }

                //Contribution (1,0,0,0).
                T dx1 = dx0 - (T)1.0 - SQUISH_CONSTANT;
                T dy1 = dy0 - SQUISH_CONSTANT;
                T dz1 = dz0 - SQUISH_CONSTANT;
                T dw1 = dw0 - SQUISH_CONSTANT;
                {
                    T attn = pow2(dx1) + pow2(dy1) + pow2(dz1) + pow2(dw1);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb + 1, ysb, zsb, wsb, dx1, dy1, dz1, dw1);
                }

                //Contribution (0,1,0,0).
                T dx2 = dx0 - SQUISH_CONSTANT;
                T dy2 = dy0 - (T)1.0 - SQUISH_CONSTANT;
                T dz2 = dz1;
                T dw2 = dw1;
                {
                    T attn = pow2(dx2) + pow2(dy2) + pow2(dz2) + pow2(dw2);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb, ysb + 1, zsb, wsb, dx2, dy2, dz2, dw2);
                }

                //Contribution (0,0,1,0).
                {
                    T dx3 = dx2;
                    T dy3 = dy1;
                    T dz3 = dz0 - (T)1.0 - SQUISH_CONSTANT;
                    T dw3 = dw1;
                    T attn = pow2(dx3) + pow2(dy3) + pow2(dz3) + pow2(dw3);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb, ysb, zsb + 1, wsb, dx3, dy3, dz3, dw3);
                }

                //Contribution (0,0,0,1).
                {
                    T dx4 = dx2;
                    T dy4 = dy1;
                    T dz4 = dz1;
                    T dw4 = dw0 - (T)1.0 - SQUISH_CONSTANT;
                    T attn = pow2(dx4) + pow2(dy4) + pow2(dz4) + pow2(dw4);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb, ysb, zsb, wsb + 1, dx4, dy4, dz4, dw4);
                }

                //Contribution (1,1,0,0).
                {
                    T dx5 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    T dy5 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    T dz5 = dz0 - (SQUISH_CONSTANT * (T)2.0);
                    T dw5 = dw0 - (SQUISH_CONSTANT * (T)2.0);
                    T attn = pow2(dx5) + pow2(dy5) + pow2(dz5) + pow2(dw5);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb + 1, ysb + 1, zsb, wsb, dx5, dy5, dz5, dw5);
                }

                //Contribution (1,0,1,0).
                {
                    T dx6 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    T dy6 = dy0 - (SQUISH_CONSTANT * (T)2.0);
                    T dz6 = dz0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    T dw6 = dw0 - (SQUISH_CONSTANT * (T)2.0);
                    T attn = pow2(dx6) + pow2(dy6) + pow2(dz6) + pow2(dw6);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb + 1, ysb, zsb + 1, wsb, dx6, dy6, dz6, dw6);
                }

                //Contribution (1,0,0,1).
                {
                    T dx7 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    T dy7 = dy0 - (SQUISH_CONSTANT * (T)2.0);
                    T dz7 = dz0 - (SQUISH_CONSTANT * (T)2.0);
                    T dw7 = dw0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    T attn = pow2(dx7) + pow2(dy7) + pow2(dz7) + pow2(dw7);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb + 1, ysb, zsb, wsb + 1, dx7, dy7, dz7, dw7);
                }

                // Contribution (0,1,1,0).
                {
                    T dx8 = dx0 - (SQUISH_CONSTANT * (T)2.0);
                    T dy8 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    T dz8 = dz0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    T dw8 = dw0 - (SQUISH_CONSTANT * (T)2.0);
                    T attn = pow2(dx8) + pow2(dy8) + pow2(dz8) + pow2(dw8);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb, ysb + 1, zsb + 1, wsb, dx8, dy8, dz8, dw8);
                }

                // Contribution (0,1,0,1).
                {
                    T dx9 = dx0 - (SQUISH_CONSTANT * (T)2.0);
                    T dy9 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    T dz9 = dz0 - (SQUISH_CONSTANT * (T)2.0);
                    T dw9 = dw0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    T attn = pow2(dx9) + pow2(dy9) + pow2(dz9) + pow2(dw9);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb, ysb + 1, zsb, wsb + 1, dx9, dy9, dz9, dw9);
                }

                // Contribution (0,0,1,1).
                {
                    T dx10 = dx0 - 2 * SQUISH_CONSTANT;
                    T dy10 = dy0 - 2 * SQUISH_CONSTANT;
                    T dz10 = dz0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    T dw10 = dw0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    T attn = pow2(dx10) + pow2(dy10) + pow2(dz10) + pow2(dw10);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb, ysb, zsb + 1, wsb + 1, dx10, dy10, dz10, dw10);
                }

            }
            else {
                // Inside the second dispentachoron (rectified 4-simplex).

                OSN_BYTE aPoint, bPoint;
                T aScore, bScore;
                bool aIsBiggerSide(true), bIsBiggerSide(true);

                // Decide between (0,0,1,1) and (1,1,0,0).
                if (xins + yins < zins + wins) {
                    aPoint = 0x0C;
                    aScore = xins + yins;
                }
                else {
                    aPoint = 0x03;
                    aScore = zins + wins;
                }

                //Decide between (0,1,0,1) and (1,0,1,0).
                if (xins + zins < yins + wins) {
                    bPoint = 0x0A;
                    bScore = xins + zins;
                }
                else {
                    bPoint = 0x05;
                    bScore = yins + wins;
                }

                // The closer of (0,1,1,0) and (1,0,0,1) will replace the further of a and b, if closer.
                if (xins + wins < yins + zins) {
                    T score(xins + wins);
                    if (aScore <= bScore && score < bScore) {
                        bPoint = 0x06;
                        bScore = score;
                    }
                    else if (aScore > bScore && score < aScore) {
                        aPoint = 0x06;
                        aScore = score;
                    }
                }
                else {
                    T score(yins + zins);
                    if (aScore <= bScore && score < bScore) {
                        bPoint = 0x09;
                        bScore = score;
                    }
                    else if (aScore > bScore && score < aScore) {
                        aPoint = 0x09;
                        aScore = score;
                    }
                }

                // Decide if (0,1,1,1) is closer.
                {
                    T p1 = 3.0 - inSum + xins;
                    if (aScore <= bScore && p1 < bScore) {
                        bPoint = 0x0E;
                        bScore = p1;
                        bIsBiggerSide = false;
                    }
                    else if (aScore > bScore && p1 < aScore) {
                        aPoint = 0x0E;
                        aScore = p1;
                        aIsBiggerSide = false;
                    }
                }

                // Decide if (1,0,1,1) is closer.
                {
                    T p2 = 3.0 - inSum + yins;
                    if (aScore <= bScore && p2 < bScore) {
                        bPoint = 0x0D;
                        bScore = p2;
                        bIsBiggerSide = false;
                    }
                    else if (aScore > bScore && p2 < aScore) {
                        aPoint = 0x0D;
                        aScore = p2;
                        aIsBiggerSide = false;
                    }
                }

                // Decide if (1,1,0,1) is closer.
                {
                    T p3 = 3.0 - inSum + zins;
                    if (aScore <= bScore && p3 < bScore) {
                        bPoint = 0x0B;
                        bScore = p3;
                        bIsBiggerSide = false;
                    }
                    else if (aScore > bScore && p3 < aScore) {
                        aPoint = 0x0B;
                        aScore = p3;
                        aIsBiggerSide = false;
                    }
                }

                // Decide if (1,1,1,0) is closer.
                {
                    T p4 = 3.0 - inSum + wins;
                    if (aScore <= bScore && p4 < bScore) {
                        bPoint = 0x07;
                        bScore = p4;
                        bIsBiggerSide = false;
                    }
                    else if (aScore > bScore && p4 < aScore) {
                        aPoint = 0x07;
                        aScore = p4;
                        aIsBiggerSide = false;
                    }
                }

                // Where each of the two closest points are determines how the extra three vertices are calculated.
                if (aIsBiggerSide == bIsBiggerSide) {
                    if (aIsBiggerSide) {
                        // Both closest points are on the bigger side.

                        OSN_BYTE c1 = aPoint & bPoint;
                        OSN_BYTE c2 = aPoint | bPoint;

                        // Two contributions are permutations of (0,0,0,1) and (0,0,0,2) based on c1.
                        xsv_ext0 = xsv_ext1 = xsb;
                        ysv_ext0 = ysv_ext1 = ysb;
                        zsv_ext0 = zsv_ext1 = zsb;
                        wsv_ext0 = wsv_ext1 = wsb;
                        dx_ext0 = dx0 - SQUISH_CONSTANT;
                        dy_ext0 = dy0 - SQUISH_CONSTANT;
                        dz_ext0 = dz0 - SQUISH_CONSTANT;
                        dw_ext0 = dw0 - SQUISH_CONSTANT;
                        dx_ext1 = dx0 - (SQUISH_CONSTANT * (T)2.0);
                        dy_ext1 = dy0 - (SQUISH_CONSTANT * (T)2.0);
                        dz_ext1 = dz0 - (SQUISH_CONSTANT * (T)2.0);
                        dw_ext1 = dw0 - (SQUISH_CONSTANT * (T)2.0);

                        if (c1 & 0x01) {
                            xsv_ext0 += 1;
                            dx_ext0 -= (T)1.0;
                            xsv_ext1 += 2;
                            dx_ext1 -= (T)2.0;
                        }
                        else if (c1 & 0x02) {
                            ysv_ext0 += 1;
                            dy_ext0 -= (T)1.0;
                            ysv_ext1 += 2;
                            dy_ext1 -= (T)2.0;
                        }
                        else if (c1 & 0x04) {
                            zsv_ext0 += 1;
                            dz_ext0 -= (T)1.0;
                            zsv_ext1 += 2;
                            dz_ext1 -= (T)2.0;
                        }
                        else {
                            wsv_ext0 += 1;
                            dw_ext0 -= (T)1.0;
                            wsv_ext1 += 2;
                            dw_ext1 -= (T)2.0;
                        }

                        // One contribution is a permutation of (1,1,1,-1) based on c2.
                        xsv_ext2 = xsb + 1;
                        ysv_ext2 = ysb + 1;
                        zsv_ext2 = zsb + 1;
                        wsv_ext2 = wsb + 1;
                        dx_ext2 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                        dy_ext2 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                        dz_ext2 = dz0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                        dw_ext2 = dw0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                        if (!(c2 & 0x01)) {
                            xsv_ext2 -= 2;
                            dx_ext2 += (T)2.0;
                        }
                        else if (!(c2 & 0x02)) {
                            ysv_ext2 -= 2;
                            dy_ext2 += (T)2.0;
                        }
                        else if (!(c2 & 0x04)) {
                            zsv_ext2 -= 2;
                            dz_ext2 += (T)2.0;
                        }
                        else {
                            wsv_ext2 -= 2;
                            dw_ext2 += (T)2.0;
                        }
                    }
                    else {
                        // Both closest points are on the smaller side.

                        // One of the two extra points is (1,1,1,1).
                        xsv_ext2 = xsb + 1;
                        ysv_ext2 = ysb + 1;
                        zsv_ext2 = zsb + 1;
                        wsv_ext2 = wsb + 1;
                        dx_ext2 = dx0 - (T)1.0 - (SQUISH_CONSTANT * 4);
                        dy_ext2 = dy0 - (T)1.0 - (SQUISH_CONSTANT * 4);
                        dz_ext2 = dz0 - (T)1.0 - (SQUISH_CONSTANT * 4);
                        dw_ext2 = dw0 - (T)1.0 - (SQUISH_CONSTANT * 4);

                        // The other two points are based on the shared axes.
                        OSN_BYTE c = aPoint & bPoint;
                        if (c & 0x01) {
                            xsv_ext0 = xsb + 2;
                            xsv_ext1 = xsb + 1;
                            dx_ext0 = dx0 - (T)2.0 - (SQUISH_CONSTANT * (T)3.0);
                            dx_ext1 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                        }
                        else {
                            xsv_ext0 = xsv_ext1 = xsb;
                            dx_ext0 = dx_ext1 = dx0 - (SQUISH_CONSTANT * (T)3.0);
                        }

                        if (c & 0x02) {
                            ysv_ext0 = ysv_ext1 = ysb + 1;
                            dy_ext0 = dy_ext1 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                            if (!(c & 0x01)) {
                                ysv_ext0 += 1;
                                dy_ext0 -= (T)1.0;
                            }
                            else {
                                ysv_ext1 += 1;
                                dy_ext1 -= (T)1.0;
                            }
                        }
                        else {
                            ysv_ext0 = ysv_ext1 = ysb;
                            dy_ext0 = dy_ext1 = dy0 - (SQUISH_CONSTANT * (T)3.0);
                        }

                        if (c & 0x04) {
                            zsv_ext0 = zsv_ext1 = zsb + 1;
                            dz_ext0 = dz_ext1 = dz0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                            if (!(c & 0x03)) {
                                zsv_ext0 += 1;
                                dz_ext0 -= (T)1.0;
                            }
                            else {
                                zsv_ext1 += 1;
                                dz_ext1 -= (T)1.0;
                            }
                        }
                        else {
                            zsv_ext0 = zsv_ext1 = zsb;
                            dz_ext0 = dz_ext1 = dz0 - (SQUISH_CONSTANT * (T)3.0);
                        }

                        if (c & 0x08) {
                            wsv_ext0 = wsb + 1;
                            wsv_ext1 = wsb + 2;
                            dw_ext0 = dw0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                            dw_ext1 = dw0 - (T)2.0 - (SQUISH_CONSTANT * (T)3.0);
                        }
                        else {
                            wsv_ext0 = wsv_ext1 = wsb;
                            dw_ext0 = dw_ext1 = dw0 - (SQUISH_CONSTANT * (T)3.0);
                        }
                    }
                }
                else {
                    // One point on each "side".

                    OSN_BYTE c1, c2;
                    if (aIsBiggerSide) {
                        c1 = aPoint;
                        c2 = bPoint;
                    }
                    else {
                        c1 = bPoint;
                        c2 = aPoint;
                    }

                    // Two contributions are the bigger-sided point with each 1 replaced with 2.
                    if (c1 & 0x01) {
                        xsv_ext0 = xsb + 2;
                        xsv_ext1 = xsb + 1;
                        dx_ext0 = dx0 - (T)2.0 - (SQUISH_CONSTANT * (T)3.0);
                        dx_ext1 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                    }
                    else {
                        xsv_ext0 = xsv_ext1 = xsb;
                        dx_ext0 = dx_ext1 = dx0 - (SQUISH_CONSTANT * (T)3.0);
                    }

                    if (c1 & 0x02) {
                        ysv_ext0 = ysv_ext1 = ysb + 1;
                        dy_ext0 = dy_ext1 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                        if (!(c1 & 0x01)) {
                            ysv_ext0 += 1;
                            dy_ext0 -= (T)1.0;
                        }
                        else {
                            ysv_ext1 += 1;
                            dy_ext1 -= (T)1.0;
                        }
                    }
                    else {
                        ysv_ext0 = ysv_ext1 = ysb;
                        dy_ext0 = dy_ext1 = dy0 - (SQUISH_CONSTANT * (T)3.0);
                    }

                    if (c1 & 0x04) {
                        zsv_ext0 = zsv_ext1 = zsb + 1;
                        dz_ext0 = dz_ext1 = dz0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                        if (!(c1 & 0x03)) {
                            zsv_ext0 += 1;
                            dz_ext0 -= (T)1.0;
                        }
                        else {
                            zsv_ext1 += 1;
                            dz_ext1 -= (T)1.0;
                        }
                    }
                    else {
                        zsv_ext0 = zsv_ext1 = zsb;
                        dz_ext0 = dz_ext1 = dz0 - 3 * (SQUISH_CONSTANT * (T)3.0);
                    }

                    if (c1 & 0x08) {
                        wsv_ext0 = wsb + 1;
                        wsv_ext1 = wsb + 2;
                        dw_ext0 = dw0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                        dw_ext1 = dw0 - (T)2.0 - (SQUISH_CONSTANT * (T)3.0);
                    }
                    else {
                        wsv_ext0 = wsv_ext1 = wsb;
                        dw_ext0 = dw_ext1 = dw0 - (SQUISH_CONSTANT * (T)3.0);
                    }

                    //  One contribution is a permutation of (1,1,1,-1) based on the smaller-sided point.
                    xsv_ext2 = xsb + 1;
                    ysv_ext2 = ysb + 1;
                    zsv_ext2 = zsb + 1;
                    wsv_ext2 = wsb + 1;
                    dx_ext2 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    dy_ext2 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    dz_ext2 = dz0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    dw_ext2 = dw0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    if (!(c2 & 0x01)) {
                        xsv_ext2 -= 2;
                        dx_ext2 += (T)2.0;
                    }
                    else if (!(c2 & 0x02)) {
                        ysv_ext2 -= 2;
                        dy_ext2 += (T)2.0;
                    }
                    else if (!(c2 & 0x04)) {
                        zsv_ext2 -= 2;
                        dz_ext2 += (T)2.0;
                    }
                    else {
                        wsv_ext2 -= 2;
                        dw_ext2 += (T)2.0;
                    }
                }

                // Contribution (1,1,1,0).
                T dx4 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                T dy4 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                T dz4 = dz0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                T dw4 = dw0 - (SQUISH_CONSTANT * (T)3.0);
                {
                    T attn = pow2(dx4) + pow2(dy4) + pow2(dz4) + pow2(dw4);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb + 1, ysb + 1, zsb + 1, wsb, dx4, dy4, dz4, dw4);
                }

                //Contribution (1,1,0,1).
                T dx3 = dx4;
                T dy3 = dy4;
                T dz3 = dz0 - (SQUISH_CONSTANT * (T)3.0);
                T dw3 = dw0 - (T)1.0 - (SQUISH_CONSTANT * (T)3.0);
                {
                    T attn = pow2(dx3) + pow2(dy3) + pow2(dz3) + pow2(dw3);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb + 1, ysb + 1, zsb, wsb + 1, dx3, dy3, dz3, dw3);
                }

                // Contribution (1,0,1,1).
                {
                    T dx2 = dx4;
                    T dy2 = dy0 - (SQUISH_CONSTANT * (T)3.0);
                    T dz2 = dz4;
                    T dw2 = dw3;
                    T attn = pow2(dx2) + pow2(dy2) + pow2(dz2) + pow2(dw2);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb + 1, ysb, zsb + 1, wsb + 1, dx2, dy2, dz2, dw2);
                }

                // Contribution (0,1,1,1).
                {
                    T dx1 = dx0 - (SQUISH_CONSTANT * (T)3.0);
                    T dz1 = dz4;
                    T dy1 = dy4;
                    T dw1 = dw3;
                    T attn = pow2(dx1) + pow2(dy1) + pow2(dz1) + pow2(dw1);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb, ysb + 1, zsb + 1, wsb + 1, dx1, dy1, dz1, dw1);
                }

                // Contribution (1,1,0,0).
                {
                    T dx5 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    T dy5 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    T dz5 = dz0 - (SQUISH_CONSTANT * (T)2.0);
                    T dw5 = dw0 - (SQUISH_CONSTANT * (T)2.0);
                    T attn = pow2(dx5) + pow2(dy5) + pow2(dz5) + pow2(dw5);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb + 1, ysb + 1, zsb, wsb, dx5, dy5, dz5, dw5);
                }

                // Contribution (1,0,1,0).
                {
                    T dx6 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    T dy6 = dy0 - (SQUISH_CONSTANT * (T)2.0);
                    T dz6 = dz0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    T dw6 = dw0 - (SQUISH_CONSTANT * (T)2.0);
                    T attn = pow2(dx6) + pow2(dy6) + pow2(dz6) + pow2(dw6);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb + 1, ysb, zsb + 1, wsb, dx6, dy6, dz6, dw6);
                }

                // Contribution (1,0,0,1).
                {
                    T dx7 = dx0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    T dy7 = dy0 - (SQUISH_CONSTANT * (T)2.0);
                    T dz7 = dz0 - (SQUISH_CONSTANT * (T)2.0);
                    T dw7 = dw0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    T attn = pow2(dx7) + pow2(dy7) + pow2(dz7) + pow2(dw7);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb + 1, ysb, zsb, wsb + 1, dx7, dy7, dz7, dw7);
                }

                // Contribution (0,1,1,0).
                {
                    T dx8 = dx0 - (SQUISH_CONSTANT * (T)2.0);
                    T dy8 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    T dz8 = dz0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    T dw8 = dw0 - (SQUISH_CONSTANT * (T)2.0);
                    T attn = pow2(dx8) + pow2(dy8) + pow2(dz8) + pow2(dw8);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb, ysb + 1, zsb + 1, wsb, dx8, dy8, dz8, dw8);
                }

                // Contribution (0,1,0,1).
                {
                    T dx9 = dx0 - (SQUISH_CONSTANT * (T)2.0);
                    T dy9 = dy0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    T dz9 = dz0 - (SQUISH_CONSTANT * (T)2.0);
                    T dw9 = dw0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    T attn = pow2(dx9) + pow2(dy9) + pow2(dz9) + pow2(dw9);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb, ysb + 1, zsb, wsb + 1, dx9, dy9, dz9, dw9);
                }

                // Contribution (0,0,1,1).
                {
                    T dx10 = dx0 - (SQUISH_CONSTANT * (T)2.0);
                    T dy10 = dy0 - (SQUISH_CONSTANT * (T)2.0);
                    T dz10 = dz0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    T dw10 = dw0 - (T)1.0 - (SQUISH_CONSTANT * (T)2.0);
                    T attn = pow2(dx10) + pow2(dy10) + pow2(dz10) + pow2(dw10);
                    value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsb, ysb, zsb + 1, wsb + 1, dx10, dy10, dz10, dw10);
                }
            }

            // First extra vertex.
            {
                T attn = pow2(dx_ext0) + pow2(dy_ext0) + pow2(dz_ext0) + pow2(dw_ext0);
                value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsv_ext0, ysv_ext0, zsv_ext0, wsv_ext0, dx_ext0, dy_ext0, dz_ext0, dw_ext0);
            }

            // Second extra vertex.
            {
                T attn = pow2(dx_ext1) + pow2(dy_ext1) + pow2(dz_ext1) + pow2(dw_ext1);
                value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsv_ext1, ysv_ext1, zsv_ext1, wsv_ext1, dx_ext1, dy_ext1, dz_ext1, dw_ext1);
            }

            // Third extra vertex.
            {
                T attn = pow2(dx_ext2) + pow2(dy_ext2) + pow2(dz_ext2) + pow2(dw_ext2);
                value += pow4(std::max((T)2.0 - attn, (T)0.0)) * extrapolate(xsv_ext2, ysv_ext2, zsv_ext2, wsv_ext2, dx_ext2, dy_ext2, dz_ext2, dw_ext2);
            }

            return (value * NORM_CONSTANT);
        }

    };


    // Array of gradient values for 4D. They approximate the directions to the
    // vertices of a disprismatotesseractihexadecachoron from its center, skewed so that the
    // tetrahedral and cubic facets can be inscribed in spheres of the same radius.
    // Gradient set 2014-10-06.
    const int Noise<4>::gradients[] = {
       3, 1, 1, 1,   1, 3, 1, 1,   1, 1, 3, 1,   1, 1, 1, 3,
      -3, 1, 1, 1,  -1, 3, 1, 1,  -1, 1, 3, 1,  -1, 1, 1, 3,
       3,-1, 1, 1,   1,-3, 1, 1,   1,-1, 3, 1,   1,-1, 1, 3,
      -3,-1, 1, 1,  -1,-3, 1, 1,  -1,-1, 3, 1,  -1,-1, 1, 3,
       3, 1,-1, 1,   1, 3,-1, 1,   1, 1,-3, 1,   1, 1,-1, 3,
      -3, 1,-1, 1,  -1, 3,-1, 1,  -1, 1,-3, 1,  -1, 1,-1, 3,
       3,-1,-1, 1,   1,-3,-1, 1,   1,-1,-3, 1,   1,-1,-1, 3,
      -3,-1,-1, 1,  -1,-3,-1, 1,  -1,-1,-3, 1,  -1,-1,-1, 3,
       3, 1, 1,-1,   1, 3, 1,-1,   1, 1, 3,-1,   1, 1, 1,-3,
      -3, 1, 1,-1,  -1, 3, 1,-1,  -1, 1, 3,-1,  -1, 1, 1,-3,
       3,-1, 1,-1,   1,-3, 1,-1,   1,-1, 3,-1,   1,-1, 1,-3,
      -3,-1, 1,-1,  -1,-3, 1,-1,  -1,-1, 3,-1,  -1,-1, 1,-3,
       3, 1,-1,-1,   1, 3,-1,-1,   1, 1,-3,-1,   1, 1,-1,-3,
      -3, 1,-1,-1,  -1, 3,-1,-1,  -1, 1,-3,-1,  -1, 1,-1,-3,
       3,-1,-1,-1,   1,-3,-1,-1,   1,-1,-3,-1,   1,-1,-1,-3,
      -3,-1,-1,-1,  -1,-3,-1,-1,  -1,-1,-3,-1,  -1,-1,-1,-3
    };

}

#else
#pragma message("OpenSimplexNoise.hh included multiple times")
#endif