#pragma once

//============================================
// FVector and FMatrix math
//============================================

inline FVector operator* (const FVector& vec, float multiplier)
{
    return FVector{ vec.X * multiplier, vec.Y * multiplier, vec.Z * multiplier };
}

inline FVector operator/ (const FVector& vec, float divisor)
{
    return FVector{ vec.X / divisor, vec.Y / divisor, vec.Z / divisor };
}

inline FVector operator+ (const FVector& vec, float multiplier)
{
    return FVector{ vec.X + multiplier, vec.Y + multiplier, vec.Z + multiplier };
}

inline FVector operator- (const FVector& vec, float divisor)
{
    return FVector{ vec.X - divisor, vec.Y - divisor, vec.Z - divisor };
}

inline FVector& operator*= (FVector& vec, float multiplier)
{
    vec.X *= multiplier;
    vec.Y *= multiplier;
    vec.Z *= multiplier;
    return vec;
}

inline FVector& operator/= (FVector& vec, float divisor)
{
    vec.X /= divisor;
    vec.Y /= divisor;
    vec.Z /= divisor;
    return vec;
}

inline FVector& operator+= (FVector& vec, float multiplier)
{
    vec.X += multiplier;
    vec.Y += multiplier;
    vec.Z += multiplier;
    return vec;
}

inline FVector& operator-= (FVector& vec, float divisor)
{
    vec.X -= divisor;
    vec.Y -= divisor;
    vec.Z -= divisor;
    return vec;
}

inline FVector operator+ (const FVector& vec, FVector vec2)
{
    return FVector{ vec.X + vec2.X, vec.Y + vec2.Y, vec.Z + vec2.Z };
}

inline FVector operator- (const FVector& vec, FVector vec2)
{
    return FVector{ vec.X - vec2.X, vec.Y - vec2.Y, vec.Z - vec2.Z };
}

inline FVector& operator+= (FVector& vec, FVector vec2)
{
    vec.X += vec2.X;
    vec.Y += vec2.Y;
    vec.Z += vec2.Z;
    return vec;
}

inline FVector& operator-= (FVector& vec, FVector vec2)
{
    vec.X -= vec2.X;
    vec.Y -= vec2.Y;
    vec.Z -= vec2.Z;
    return vec;
}

inline float Dot(FVector vector1, FVector vector2)
{
    return (vector1.X * vector2.X)
        + (vector1.Y * vector2.Y)
        + (vector1.Z * vector2.Z);
}

inline FVector Cross(FVector a, FVector b)
{
    return FVector{
        a.Y * b.Z - a.Z * b.Y,
        a.Z * b.X - a.X * b.Z,
        a.X * b.Y - a.Y * b.X
    };
}

inline int DegreesToUnrealRotationUnits(float degrees)
{
    return static_cast<int>(degrees * 65536.f / 360.f);
}

inline int RadiansToUnrealRotationUnits(float radians)
{
    return static_cast<int>(radians * 180.f / 3.14159265f * 65536.f / 360.f);
}

inline float UnrealRotationUnitsToDegrees(int unrealRotationUnits)
{
    return unrealRotationUnits * 360.f / 65536.f;
}

inline float UnrealRotationUnitsToRadians(int unrealRotationUnits)
{
    return unrealRotationUnits * 360.f / 65536.f * 3.14159265f / 180.f;
}

constexpr FMatrix IdentityMatrix
{
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1}
};

inline void MatrixDecompose(FMatrix m, FVector& translation, FVector& scale, float& pitchRad, float& yawRad, float& rollRad)
{
    translation = FVector{ m.WPlane.X, m.WPlane.Y, m.WPlane.Z };

    FVector xAxis = m.XPlane;
    FVector yAxis = m.YPlane;
    FVector zAxis = m.ZPlane;

    scale = FVector{ sqrt(xAxis.X * xAxis.X + xAxis.Y * xAxis.Y + xAxis.Z * xAxis.Z),
                    sqrt(yAxis.X * yAxis.X + yAxis.Y * yAxis.Y + yAxis.Z * yAxis.Z),
                    sqrt(zAxis.X * zAxis.X + zAxis.Y * zAxis.Y + zAxis.Z * zAxis.Z) };

    constexpr float epsilon = 1e-6f;

    if (scale.X < epsilon || scale.Y < epsilon || scale.Z < epsilon)
    {
        pitchRad = yawRad = rollRad = 0;
        return;
    }

    // Detect reflection via determinant sign
    // Negative determinant = odd number of axis reflections; assign to X by convention
    float det = Dot(xAxis, Cross(yAxis, zAxis));
    if (det < 0.f)
    {
        scale.X = -scale.X;
    }

    xAxis.X /= scale.X;
    xAxis.Y /= scale.X;
    xAxis.Z /= scale.X;

    yAxis.X /= scale.Y;
    yAxis.Y /= scale.Y;
    yAxis.Z /= scale.Y;

    zAxis.X /= scale.Z;
    zAxis.Y /= scale.Z;
    zAxis.Z /= scale.Z;

    pitchRad = atan2(xAxis.Z, sqrt(xAxis.X * xAxis.X + xAxis.Y * xAxis.Y));
    yawRad = atan2(xAxis.Y, xAxis.X);

    auto syAxis = FVector{ -sin(yawRad), cos(yawRad), 0.f };

    rollRad = atan2(Dot(zAxis, syAxis), Dot(yAxis, syAxis));
}

inline FMatrix MatrixCompose(FVector translation, FVector scale, float pitchRad, float yawRad, float rollRad)
{
    float sp = sin(pitchRad);
    float sy = sin(yawRad);
    float sr = sin(rollRad);
    float cp = cos(pitchRad);
    float cy = cos(yawRad);
    float cr = cos(rollRad);

    float sX = scale.X;
    float sY = scale.Y;
    float sZ = scale.Z;

    return FMatrix
    {
        {
            cp * cy * sX,
            cp * sX * sy,
            sX * sp,
            0.f
        },
        {
            sY * (cy * sp * sr - cr * sy),
            sY * (cr * cy + sp * sr * sy),
            -cp * sY * sr,
            0.f
        },
        {
            -sZ * (cr * cy * sp + sr * sy),
            sZ * (cy * sr - cr * sp * sy),
            cp * cr * sZ,
            0.f
        },
        {
            translation,
            1.f
        }
    };
}

inline FMatrix operator* (const FMatrix& matrix1, const FMatrix& matrix2)
{
    typedef float MatrixAs2DArray[4][4];
    MatrixAs2DArray& m1 = *((MatrixAs2DArray*)&matrix1);
    MatrixAs2DArray& m2 = *((MatrixAs2DArray*)&matrix2);
    FMatrix result;
    MatrixAs2DArray& r = *((MatrixAs2DArray*)&result);
    r[0][0] = m1[0][0] * m2[0][0] + m1[0][1] * m2[1][0] + m1[0][2] * m2[2][0] + m1[0][3] * m2[3][0];
    r[0][1] = m1[0][0] * m2[0][1] + m1[0][1] * m2[1][1] + m1[0][2] * m2[2][1] + m1[0][3] * m2[3][1];
    r[0][2] = m1[0][0] * m2[0][2] + m1[0][1] * m2[1][2] + m1[0][2] * m2[2][2] + m1[0][3] * m2[3][2];
    r[0][3] = m1[0][0] * m2[0][3] + m1[0][1] * m2[1][3] + m1[0][2] * m2[2][3] + m1[0][3] * m2[3][3];

    r[1][0] = m1[1][0] * m2[0][0] + m1[1][1] * m2[1][0] + m1[1][2] * m2[2][0] + m1[1][3] * m2[3][0];
    r[1][1] = m1[1][0] * m2[0][1] + m1[1][1] * m2[1][1] + m1[1][2] * m2[2][1] + m1[1][3] * m2[3][1];
    r[1][2] = m1[1][0] * m2[0][2] + m1[1][1] * m2[1][2] + m1[1][2] * m2[2][2] + m1[1][3] * m2[3][2];
    r[1][3] = m1[1][0] * m2[0][3] + m1[1][1] * m2[1][3] + m1[1][2] * m2[2][3] + m1[1][3] * m2[3][3];

    r[2][0] = m1[2][0] * m2[0][0] + m1[2][1] * m2[1][0] + m1[2][2] * m2[2][0] + m1[2][3] * m2[3][0];
    r[2][1] = m1[2][0] * m2[0][1] + m1[2][1] * m2[1][1] + m1[2][2] * m2[2][1] + m1[2][3] * m2[3][1];
    r[2][2] = m1[2][0] * m2[0][2] + m1[2][1] * m2[1][2] + m1[2][2] * m2[2][2] + m1[2][3] * m2[3][2];
    r[2][3] = m1[2][0] * m2[0][3] + m1[2][1] * m2[1][3] + m1[2][2] * m2[2][3] + m1[2][3] * m2[3][3];

    r[3][0] = m1[3][0] * m2[0][0] + m1[3][1] * m2[1][0] + m1[3][2] * m2[2][0] + m1[3][3] * m2[3][0];
    r[3][1] = m1[3][0] * m2[0][1] + m1[3][1] * m2[1][1] + m1[3][2] * m2[2][1] + m1[3][3] * m2[3][1];
    r[3][2] = m1[3][0] * m2[0][2] + m1[3][1] * m2[1][2] + m1[3][2] * m2[2][2] + m1[3][3] * m2[3][2];
    r[3][3] = m1[3][0] * m2[0][3] + m1[3][1] * m2[1][3] + m1[3][2] * m2[2][3] + m1[3][3] * m2[3][3];
    return result;
}
