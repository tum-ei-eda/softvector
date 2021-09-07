/*
 * Copyright [2020] [Technical University of Munich]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//////////////////////////////////////////////////////////////////////////////////////
/// \file softvector-register.cpp
/// \brief Extended register softvector representation
/// \date 07/03/2020
//////////////////////////////////////////////////////////////////////////////////////

#include "vpu/softvector-types.hpp"

SVRegister& SVRegister::operator=(const SVRegister& rhs) {
	for (size_t i_byte = 0; i_byte < length_bits_/8; ++i_byte) {
		(*this)[i_byte] = rhs[i_byte];
	}
	return (*this);
}

SVRegister& SVRegister::operator=(const int64_t rhs) {
	for (size_t i_byte = 0; i_byte < length_bits_; ++i_byte) {
		if(i_byte < 8)
			(*this)[i_byte] = rhs >> 8*i_byte;
		else
			(*this)[i_byte] = (rhs >= 0) ? 0 : 0xFF;
	}
	return (*this);
}

void SVRegister::m_assign(
	const SVRegister& in,
	const SVRegister& vm,
	size_t start_index
) {
	for(size_t i_byte = start_index/8; i_byte < length_bits_; ++i_byte) {
		if((i_byte == start_index/8) and (start_index != 0)) {
			for(size_t i_bit = start_index % 8; i_bit <8; ++i_bit) {
				(*this)[i_byte] = ((*this)[i_byte] & (~vm[i_byte] & (0x1 << i_bit))) ^ (in[i_byte] & vm[i_byte]  & (0x1 << i_bit));
			}
		}
		else
			(*this)[i_byte] = ((*this)[i_byte] & (~vm[i_byte])) ^ (in[i_byte] & vm[i_byte]);
	}
}

SVRegister& SVRegister::m_eq(const SVector& opL, const SVector& rhs,  const SVRegister& vm, bool mask, size_t start_index) {
	for(size_t i_element = start_index; i_element < opL.length_; ++i_element) {
		if(!mask or vm.get_bit(i_element))
			set_bit(i_element, opL[i_element] == rhs[i_element]);
	}
	return(*this);
}

SVRegister& SVRegister::m_eq(const SVector& opL, const int64_t rhs, const SVRegister& vm, bool mask, size_t start_index ) {
	for(size_t i_element = start_index; i_element < opL.length_; ++i_element) {
		if(!mask or vm.get_bit(i_element))
			set_bit(i_element, opL[i_element] == rhs);
	}
	return(*this);
}

SVRegister& SVRegister::m_neq(const SVector& opL, const SVector& rhs,  const SVRegister& vm, bool mask, size_t start_index) {
	for(size_t i_element = start_index; i_element < opL.length_; ++i_element) {
		if(!mask or vm.get_bit(i_element))
			set_bit(i_element, opL[i_element] != rhs[i_element]);
	}
	return(*this);
}

SVRegister& SVRegister::m_neq(const SVector& opL, const int64_t rhs, const SVRegister& vm, bool mask, size_t start_index ) {
	for(size_t i_element = start_index; i_element < opL.length_; ++i_element) {
		if(!mask or vm.get_bit(i_element))
			set_bit(i_element, opL[i_element] != rhs);
	}
	return(*this);
}

SVRegister& SVRegister::m_s_lt(const SVector& opL, const SVector& rhs,  const SVRegister& vm, bool mask, size_t start_index) {
	for(size_t i_element = start_index; i_element < opL.length_; ++i_element) {
		if(!mask or vm.get_bit(i_element))
			set_bit(i_element, opL[i_element] < rhs[i_element]);
	}
	return(*this);
}

SVRegister& SVRegister::m_s_lt(const SVector& opL, const int64_t rhs, const SVRegister& vm, bool mask, size_t start_index ) {
	for(size_t i_element = start_index; i_element < opL.length_; ++i_element) {
		if(!mask or vm.get_bit(i_element))
			set_bit(i_element, opL[i_element] < rhs);
	}
	return(*this);
}

SVRegister& SVRegister::m_s_lte(const SVector& opL, const SVector& rhs,  const SVRegister& vm, bool mask, size_t start_index) {
	for(size_t i_element = start_index; i_element < opL.length_; ++i_element) {
		if(!mask or vm.get_bit(i_element))
			set_bit(i_element, opL[i_element] <= rhs[i_element]);
	}
	return(*this);
}

SVRegister& SVRegister::m_s_lte(const SVector& opL, const int64_t rhs, const SVRegister& vm, bool mask, size_t start_index ) {
	for(size_t i_element = start_index; i_element < opL.length_; ++i_element) {
		if(!mask or vm.get_bit(i_element))
			set_bit(i_element, opL[i_element] <= rhs);
	}
	return(*this);
}

SVRegister& SVRegister::m_s_gt(const SVector& opL, const SVector& rhs,  const SVRegister& vm, bool mask, size_t start_index) {
	for(size_t i_element = start_index; i_element < opL.length_; ++i_element) {
		if(!mask or vm.get_bit(i_element))
			set_bit(i_element, opL[i_element] > rhs[i_element]);
	}
	return(*this);
}

SVRegister& SVRegister::m_s_gt(const SVector& opL, const int64_t rhs, const SVRegister& vm, bool mask, size_t start_index ) {
	for(size_t i_element = start_index; i_element < opL.length_; ++i_element) {
		if(!mask or vm.get_bit(i_element))
			set_bit(i_element, opL[i_element] > rhs);
	}
	return(*this);
}

SVRegister& SVRegister::m_s_gte(const SVector& opL, const SVector& rhs,  const SVRegister& vm, bool mask, size_t start_index) {
	for(size_t i_element = start_index; i_element < opL.length_; ++i_element) {
		if(!mask or vm.get_bit(i_element))
			set_bit(i_element, opL[i_element] >= rhs[i_element]);
	}
	return(*this);
}

SVRegister& SVRegister::m_s_gte(const SVector& opL, const int64_t rhs, const SVRegister& vm, bool mask, size_t start_index ) {
	for(size_t i_element = start_index; i_element < opL.length_; ++i_element) {
		if(!mask or vm.get_bit(i_element))
			set_bit(i_element, opL[i_element] >= rhs);
	}
	return(*this);
}

// unsigned comparisons
SVRegister& SVRegister::m_u_lt(const SVector& opL, const SVector& rhs,  const SVRegister& vm, bool mask, size_t start_index) {
	for(size_t i_element = start_index; i_element < opL.length_; ++i_element) {
		if(!mask or vm.get_bit(i_element))
			set_bit(i_element, opL[i_element].op_u_lt(rhs[i_element]));
	}
	return(*this);
}

SVRegister& SVRegister::m_u_lt(const SVector& opL, const uint64_t rhs, const SVRegister& vm, bool mask, size_t start_index ) {
	for(size_t i_element = start_index; i_element < opL.length_; ++i_element) {
		if(!mask or vm.get_bit(i_element))
			set_bit(i_element, opL[i_element].op_u_lt(rhs));
	}
	return(*this);
}

SVRegister& SVRegister::m_u_lte(const SVector& opL, const SVector& rhs,  const SVRegister& vm, bool mask, size_t start_index) {
	for(size_t i_element = start_index; i_element < opL.length_; ++i_element) {
		if(!mask or vm.get_bit(i_element))
			set_bit(i_element, opL[i_element].op_u_lte(rhs[i_element]));
	}
	return(*this);
}

SVRegister& SVRegister::m_u_lte(const SVector& opL, const uint64_t rhs, const SVRegister& vm, bool mask, size_t start_index ) {
	for(size_t i_element = start_index; i_element < opL.length_; ++i_element) {
		if(!mask or vm.get_bit(i_element))
			set_bit(i_element, opL[i_element].op_u_lte(rhs));
	}
	return(*this);
}

SVRegister& SVRegister::m_u_gt(const SVector& opL, const SVector& rhs,  const SVRegister& vm, bool mask, size_t start_index) {
	for(size_t i_element = start_index; i_element < opL.length_; ++i_element) {
		if(!mask or vm.get_bit(i_element))
			set_bit(i_element, opL[i_element].op_u_gt(rhs[i_element]));
	}
	return(*this);
}

SVRegister& SVRegister::m_u_gt(const SVector& opL, const uint64_t rhs, const SVRegister& vm, bool mask, size_t start_index ) {
	for(size_t i_element = start_index; i_element < opL.length_; ++i_element) {
		if(!mask or vm.get_bit(i_element))
			set_bit(i_element, opL[i_element].op_u_gt(rhs));
	}
	return(*this);
}

SVRegister& SVRegister::m_u_gte(const SVector& opL, const SVector& rhs,  const SVRegister& vm, bool mask, size_t start_index) {
	for(size_t i_element = start_index; i_element < opL.length_; ++i_element) {
		if(!mask or vm.get_bit(i_element))
			set_bit(i_element, opL[i_element].op_u_gte(rhs[i_element]));
	}
	return(*this);
}

SVRegister& SVRegister::m_u_gte(const SVector& opL, const uint64_t rhs, const SVRegister& vm, bool mask, size_t start_index ) {
	for(size_t i_element = start_index; i_element < opL.length_; ++i_element) {
		if(!mask or vm.get_bit(i_element))
			set_bit(i_element, opL[i_element].op_u_gte(rhs));
	}
	return(*this);
}
