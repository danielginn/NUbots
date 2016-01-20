/*
 * This file is part of the NUbots Codebase.
 *
 * The NUbots Codebase is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The NUbots Codebase is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the NUbots Codebase.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright 2015 NUBots <nubots@nubots.net>
 */

#ifndef UTILITY_CONVERSION_PROTO_ARMADILLO_H
#define UTILITY_CONVERSION_PROTO_ARMADILLO_H

#include <armadillo>
#include "utility/math/matrix/Rotation3D.h"
#include "utility/math/matrix/Rotation2D.h"
#include "utility/math/matrix/Transform3D.h"
#include "utility/math/matrix/Transform2D.h"

#include "message/Vector.pb.h"
#include "message/Matrix.pb.h"
#include "message/Rotation.pb.h"
#include "message/Transform.pb.h"

/**
 * @brief This type maps protocol buffer vector types to their armadillo equivliants
 *
 * @tparam T the protocol buffer type to map
 */
template <typename T>
struct ProtoArmaVecMap;
template <> struct ProtoArmaVecMap<protobuf::message::vec2>   { using type = arma::vec2;   };
template <> struct ProtoArmaVecMap<protobuf::message::vec3>   { using type = arma::vec3;   };
template <> struct ProtoArmaVecMap<protobuf::message::vec4>   { using type = arma::vec4;   };
template <> struct ProtoArmaVecMap<protobuf::message::fvec2>  { using type = arma::fvec2;  };
template <> struct ProtoArmaVecMap<protobuf::message::fvec3>  { using type = arma::fvec3;  };
template <> struct ProtoArmaVecMap<protobuf::message::fvec4>  { using type = arma::fvec4;  };
template <> struct ProtoArmaVecMap<protobuf::message::ivec2>  { using type = arma::ivec2;  };
template <> struct ProtoArmaVecMap<protobuf::message::ivec3>  { using type = arma::ivec3;  };
template <> struct ProtoArmaVecMap<protobuf::message::ivec4>  { using type = arma::ivec4;  };
template <> struct ProtoArmaVecMap<protobuf::message::uvec2>  { using type = arma::uvec2;  };
template <> struct ProtoArmaVecMap<protobuf::message::uvec3>  { using type = arma::uvec3;  };
template <> struct ProtoArmaVecMap<protobuf::message::uvec4>  { using type = arma::uvec4;  };
template <typename T>
using ProtoArmaVec = typename ProtoArmaVecMap<T>::type;

/**
 * @brief This type maps armadillo vector types to their protocol buffer equivliants
 *
 * @tparam T the armadillo type to map
 */
template <typename T>
struct ArmaProtoVecMap;
template <> struct ArmaProtoVecMap<arma::vec2>   { using type = protobuf::message::vec2;   };
template <> struct ArmaProtoVecMap<arma::vec3>   { using type = protobuf::message::vec3;   };
template <> struct ArmaProtoVecMap<arma::vec4>   { using type = protobuf::message::vec4;   };
template <> struct ArmaProtoVecMap<arma::fvec2>  { using type = protobuf::message::fvec2;  };
template <> struct ArmaProtoVecMap<arma::fvec3>  { using type = protobuf::message::fvec3;  };
template <> struct ArmaProtoVecMap<arma::fvec4>  { using type = protobuf::message::fvec4;  };
template <> struct ArmaProtoVecMap<arma::ivec2>  { using type = protobuf::message::ivec2;  };
template <> struct ArmaProtoVecMap<arma::ivec3>  { using type = protobuf::message::ivec3;  };
template <> struct ArmaProtoVecMap<arma::ivec4>  { using type = protobuf::message::ivec4;  };
template <> struct ArmaProtoVecMap<arma::uvec2>  { using type = protobuf::message::uvec2;  };
template <> struct ArmaProtoVecMap<arma::uvec3>  { using type = protobuf::message::uvec3;  };
template <> struct ArmaProtoVecMap<arma::uvec4>  { using type = protobuf::message::uvec4;  };
template <typename T>
using ArmaProtoVec = typename ArmaProtoVecMap<T>::type;

/**
 * @brief This type maps protocol buffer matrix types to their armadillo equivliants
 *
 * @tparam T the protocol buffer type to map
 */
template <typename T>
struct ProtoArmaMatMap;
template <> struct ProtoArmaMatMap<protobuf::message::mat22>  { using type = arma::mat22;  };
template <> struct ProtoArmaMatMap<protobuf::message::mat33>  { using type = arma::mat33;  };
template <> struct ProtoArmaMatMap<protobuf::message::mat44>  { using type = arma::mat44;  };
template <> struct ProtoArmaMatMap<protobuf::message::fmat22> { using type = arma::fmat22; };
template <> struct ProtoArmaMatMap<protobuf::message::fmat33> { using type = arma::fmat33; };
template <> struct ProtoArmaMatMap<protobuf::message::fmat44> { using type = arma::fmat44; };
template <> struct ProtoArmaMatMap<protobuf::message::imat22> { using type = arma::imat22; };
template <> struct ProtoArmaMatMap<protobuf::message::imat33> { using type = arma::imat33; };
template <> struct ProtoArmaMatMap<protobuf::message::imat44> { using type = arma::imat44; };
template <> struct ProtoArmaMatMap<protobuf::message::umat22> { using type = arma::umat22; };
template <> struct ProtoArmaMatMap<protobuf::message::umat33> { using type = arma::umat33; };
template <> struct ProtoArmaMatMap<protobuf::message::umat44> { using type = arma::umat44; };
template <typename T>
using ProtoArmaMat = typename ProtoArmaMatMap<T>::type;

/**
 * @brief This type maps armadillo matrix types to their protocol buffer equivliants
 *
 * @tparam T the armadillo type to map
 */
template <typename T>
struct ArmaProtoMatMap;
template <> struct ArmaProtoMatMap<arma::mat22>  { using type = protobuf::message::mat22;  };
template <> struct ArmaProtoMatMap<arma::mat33>  { using type = protobuf::message::mat33;  };
template <> struct ArmaProtoMatMap<arma::mat44>  { using type = protobuf::message::mat44;  };
template <> struct ArmaProtoMatMap<arma::fmat22> { using type = protobuf::message::fmat22; };
template <> struct ArmaProtoMatMap<arma::fmat33> { using type = protobuf::message::fmat33; };
template <> struct ArmaProtoMatMap<arma::fmat44> { using type = protobuf::message::fmat44; };
template <> struct ArmaProtoMatMap<arma::imat22> { using type = protobuf::message::imat22; };
template <> struct ArmaProtoMatMap<arma::imat33> { using type = protobuf::message::imat33; };
template <> struct ArmaProtoMatMap<arma::imat44> { using type = protobuf::message::imat44; };
template <> struct ArmaProtoMatMap<arma::umat22> { using type = protobuf::message::umat22; };
template <> struct ArmaProtoMatMap<arma::umat33> { using type = protobuf::message::umat33; };
template <> struct ArmaProtoMatMap<arma::umat44> { using type = protobuf::message::umat44; };
template <typename T>
using ArmaProtoMat = typename ArmaProtoMatMap<T>::type;

template <typename Proto>
Proto& operator<< (Proto& proto, const ProtoArmaVec<Proto>& vec);
template <typename Vector>
Vector& operator<< (Vector& vector, const ArmaProtoVec<Vector>& proto);
template <typename Proto>
Proto& operator<< (Proto& proto, const ProtoArmaMat<Proto>& mat);
template <typename Matrix>
Matrix& operator<< (Matrix& matrix, const ArmaProtoMat<Matrix>& proto);

/**
 * @brief SFINAE functions to set protocol buffer values
 */
namespace proto_set_vec {
    template <typename Proto, typename Vector>
    inline auto x(Proto& proto, const Vector& vector) -> decltype(proto.x(), void()) {
        proto.set_x(vector[0]);
    }
    inline void x(...) {}

    template <typename Proto, typename Vector>
    inline auto y(Proto& proto, const Vector& vector) -> decltype(proto.y(), void()) {
        proto.set_y(vector[1]);
    }
    inline void y(...) {}

    template <typename Proto, typename Vector>
    inline auto z(Proto& proto, const Vector& vector) -> decltype(proto.z(), void()) {
        proto.set_z(vector[2]);
    }
    inline void z(...) {}

    template <typename Proto, typename Vector>
    inline auto t(Proto& proto, const Vector& vector) -> decltype(proto.t(), void()) {
        proto.set_t(vector[3]);
    }
    inline void t(...) {}
}

/**
 * @brief SFINAE functions to set armadillo values
 */
namespace arma_set_vec {
    template <typename Proto, typename Vector>
    inline auto x(Vector& vector, const Proto& proto) -> decltype(proto.x(), void()) {
        vector[0] = proto.x();
    }
    inline void x(...) {}

    template <typename Proto, typename Vector>
    inline auto y(Vector& vector, const Proto& proto) -> decltype(proto.y(), void()) {
        vector[1] = proto.y();
    }
    inline void y(...) {}

    template <typename Proto, typename Vector>
    inline auto z(Vector& vector, const Proto& proto) -> decltype(proto.z(), void()) {
        vector[2] = proto.z();
    }
    inline void z(...) {}

    template <typename Proto, typename Vector>
    inline auto t(Vector& vector, const Proto& proto) -> decltype(proto.t(), void()) {
        vector[3] = proto.t();
    }
    inline void t(...) {}
}

/**
 * @brief SFINAE functions to set protocol buffer values
 */
namespace proto_set_mat {
    template <typename Proto, typename Matrix>
    inline auto x(Proto& proto, const Matrix& matrix) -> decltype(proto.x(), void()) {
        proto_set_vec::x(*proto.mutable_x(), matrix.col(0));
        proto_set_vec::y(*proto.mutable_x(), matrix.col(0));
        proto_set_vec::z(*proto.mutable_x(), matrix.col(0));
        proto_set_vec::t(*proto.mutable_x(), matrix.col(0));
    }
    inline void x(...) {}

    template <typename Proto, typename Matrix>
    inline auto y(Proto& proto, const Matrix& matrix) -> decltype(proto.y(), void()) {
        proto_set_vec::x(*proto.mutable_y(), matrix.col(1));
        proto_set_vec::y(*proto.mutable_y(), matrix.col(1));
        proto_set_vec::z(*proto.mutable_y(), matrix.col(1));
        proto_set_vec::t(*proto.mutable_y(), matrix.col(1));
    }
    inline void y(...) {}

    template <typename Proto, typename Matrix>
    inline auto z(Proto& proto, const Matrix& matrix) -> decltype(proto.z(), void()) {
        proto_set_vec::x(*proto.mutable_z(), matrix.col(2));
        proto_set_vec::y(*proto.mutable_z(), matrix.col(2));
        proto_set_vec::z(*proto.mutable_z(), matrix.col(2));
        proto_set_vec::t(*proto.mutable_z(), matrix.col(2));
    }
    inline void z(...) {}

    template <typename Proto, typename Matrix>
    inline auto t(Proto& proto, const Matrix& matrix) -> decltype(proto.t(), void()) {
        proto_set_vec::x(*proto.mutable_t(), matrix.col(3));
        proto_set_vec::y(*proto.mutable_t(), matrix.col(3));
        proto_set_vec::z(*proto.mutable_t(), matrix.col(3));
        proto_set_vec::t(*proto.mutable_t(), matrix.col(3));
    }
    inline void t(...) {}
}

/**
 * @brief SFINAE functions to set armadillo values
 */
namespace arma_set_mat {
    template <typename Proto, typename Matrix>
    inline auto x(Matrix& matrix, const Proto& proto) -> decltype(proto.x(), void()) {
        arma_set_vec::x(matrix.col(0), proto.x());
        arma_set_vec::y(matrix.col(0), proto.x());
        arma_set_vec::z(matrix.col(0), proto.x());
        arma_set_vec::t(matrix.col(0), proto.x());
    }
    inline void x(...) {}

    template <typename Proto, typename Matrix>
    inline auto y(Matrix& matrix, const Proto& proto) -> decltype(proto.y(), void()) {
        arma_set_vec::x(matrix.col(1), proto.y());
        arma_set_vec::y(matrix.col(1), proto.y());
        arma_set_vec::z(matrix.col(1), proto.y());
        arma_set_vec::t(matrix.col(1), proto.y());
    }
    inline void y(...) {}

    template <typename Proto, typename Matrix>
    inline auto z(Matrix& matrix, const Proto& proto) -> decltype(proto.z(), void()) {
        arma_set_vec::x(matrix.col(2), proto.z());
        arma_set_vec::y(matrix.col(2), proto.z());
        arma_set_vec::z(matrix.col(2), proto.z());
        arma_set_vec::t(matrix.col(2), proto.z());
    }
    inline void z(...) {}

    template <typename Proto, typename Matrix>
    inline auto t(Matrix& matrix, const Proto& proto) -> decltype(proto.t(), void()) {
        arma_set_vec::x(matrix.col(3), proto.t());
        arma_set_vec::y(matrix.col(3), proto.t());
        arma_set_vec::z(matrix.col(3), proto.t());
        arma_set_vec::t(matrix.col(3), proto.t());
    }
    inline void t(...) {}
}

/**
 * @brief Generalised function for streaming armadillo vectors into protocol buffers
 *
 * @details This uses SFINAE functions to set the components of a protocol buffer.
 *          If one of the components exists, it will use the function that sets it.
 *          Otherwise it will fall back to the other empty function.
 *
 * @param proto The protocol buffer to stream the vector into
 * @param vector The vector to stream into the protocol buffer
 *
 * @return The original protocol buffer instance
 */
template <typename Proto>
inline Proto& operator<< (Proto& proto, const ProtoArmaVec<Proto>& vector) {
    proto_set_vec::x(proto, vector);
    proto_set_vec::y(proto, vector);
    proto_set_vec::z(proto, vector);
    proto_set_vec::t(proto, vector);
    return proto;
}

template <typename Proto, typename Vec>
inline Proto& set_protocol_buffer_from_armadillo_vector(Proto& proto, const Vec& vector) {
    for (auto& v : vector) {
        proto.add_v(v);
    }
    return proto;
}

inline protobuf::message::vec& operator<< (protobuf::message::vec& proto, const arma::vec& vector) {
    return set_protocol_buffer_from_armadillo_vector(proto, vector);
}

inline protobuf::message::fvec& operator<< (protobuf::message::fvec& proto, const arma::fvec& vector) {
    return set_protocol_buffer_from_armadillo_vector(proto, vector);
}

inline protobuf::message::ivec& operator<< (protobuf::message::ivec& proto, const arma::ivec& vector) {
    return set_protocol_buffer_from_armadillo_vector(proto, vector);
}

inline protobuf::message::uvec& operator<< (protobuf::message::uvec& proto, const arma::uvec& vector) {
    return set_protocol_buffer_from_armadillo_vector(proto, vector);
}

template <typename Vector>
Vector& operator<< (Vector& vector, const ArmaProtoVec<Vector>& proto) {
    arma_set_vec::x(vector, proto);
    arma_set_vec::y(vector, proto);
    arma_set_vec::z(vector, proto);
    arma_set_vec::t(vector, proto);
    return vector;
}

template <typename Vector, typename Proto>
inline Vector& set_armadillo_vector_from_protocol_buffer(Vector& vector, const Proto& proto) {
    vector.set_size(proto.v_size());
    for (uint i = 0; i < vector.n_elem; ++i) {
        vector[i] = proto.v(i);
    }
    return vector;
}

inline arma::vec& operator<< (arma::vec& vector, const protobuf::message::vec& proto) {
    return set_armadillo_vector_from_protocol_buffer(vector, proto);
}
inline arma::fvec& operator<< (arma::fvec& vector, const protobuf::message::fvec& proto) {
    return set_armadillo_vector_from_protocol_buffer(vector, proto);
}
inline arma::ivec& operator<< (arma::ivec& vector, const protobuf::message::ivec& proto) {
    return set_armadillo_vector_from_protocol_buffer(vector, proto);
}
inline arma::uvec& operator<< (arma::uvec& vector, const protobuf::message::uvec& proto) {
    return set_armadillo_vector_from_protocol_buffer(vector, proto);
}


/**
 * @brief Generalised function for streaming armadillo square matricies into protocol buffers
 *
 * @details This uses SFINAE functions to set the components of a protocol buffer.
 *          If one of the components exists, it will use the function that sets it.
 *          Otherwise it will fall back to the other empty function.
 *
 * @param proto The protocol buffer to stream the vector into
 * @param vector The vector to stream into the protocol buffer
 *
 * @return The original protocol buffer instance
 */
template <typename Proto>
inline Proto& operator<< (Proto& proto, const ProtoArmaMat<Proto>& matrix) {
    proto_set_mat::x(proto, matrix);
    proto_set_mat::y(proto, matrix);
    proto_set_mat::z(proto, matrix);
    proto_set_mat::t(proto, matrix);
    return proto;
}

template <typename Proto, typename Matrix>
inline Proto& set_protocol_buffer_from_armadillo_matrix(Proto& proto, const Matrix& matrix) {
    for (uint x = 0; x < matrix.n_cols; ++x) {
        auto col = proto.add_v();
        for (uint y = 0; y < matrix.n_rows; ++y) {
            col->add_v(matrix(y, x));
        }
    }
    return proto;
}

inline protobuf::message::mat& operator<< (protobuf::message::mat& proto, const arma::mat matrix) {
    return set_protocol_buffer_from_armadillo_matrix(proto, matrix);
}
inline protobuf::message::fmat& operator<< (protobuf::message::fmat& proto, const arma::fmat matrix) {
    return set_protocol_buffer_from_armadillo_matrix(proto, matrix);
}
inline protobuf::message::imat& operator<< (protobuf::message::imat& proto, const arma::imat matrix) {
    return set_protocol_buffer_from_armadillo_matrix(proto, matrix);
}
inline protobuf::message::umat& operator<< (protobuf::message::umat& proto, const arma::umat matrix) {
    return set_protocol_buffer_from_armadillo_matrix(proto, matrix);
}

template <typename Matrix>
Matrix& operator<< (Matrix& matrix, const ArmaProtoMat<Matrix>& proto) {
    arma_set_mat::x(matrix, proto);
    arma_set_mat::y(matrix, proto);
    arma_set_mat::z(matrix, proto);
    arma_set_mat::t(matrix, proto);
    return matrix;
}

template <typename Proto, typename Matrix>
inline Matrix& set_armadillo_matrix_from_protocol_buffer(Matrix& matrix, const Proto& proto) {
    matrix.set_size(proto.v_size(), proto.v_size());

    for (uint x = 0; x < matrix.n_cols; ++x) {
        auto& col = proto.v(x);
        for (uint y = 0; y < matrix.n_rows; ++y) {
            matrix(y, x) = col.v(y);
        }
    }
    return matrix;
}

inline arma::mat& operator<< (arma::mat& matrix, const protobuf::message::mat proto) {
    return set_armadillo_matrix_from_protocol_buffer(matrix, proto);
}
inline arma::fmat& operator<< (arma::fmat& matrix, const protobuf::message::fmat proto) {
    return set_armadillo_matrix_from_protocol_buffer(matrix, proto);
}
inline arma::imat& operator<< (arma::imat& matrix, const protobuf::message::imat proto) {
    return set_armadillo_matrix_from_protocol_buffer(matrix, proto);
}
inline arma::umat& operator<< (arma::umat& matrix, const protobuf::message::umat proto) {
    return set_armadillo_matrix_from_protocol_buffer(matrix, proto);
}


#endif  // UTILITY_CONVERSION_PROTO_ARMADILLO_H
