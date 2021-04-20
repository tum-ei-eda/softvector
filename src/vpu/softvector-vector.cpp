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
/// \file softvector-vector.cpp
/// \brief Extended vector type for softvector representation
/// \date 07/03/2020
//////////////////////////////////////////////////////////////////////////////////////

#include "vpu/softvector-types.hpp"

void SVector::assign(const SVector& vin, size_t start_index){
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		(*this)[i_element] = vin[i_element];
	}
}

SVector& SVector::operator=(const SVector& rhs){
	for (size_t i_element = 0; i_element < length_; ++i_element){
		(*this)[i_element] = rhs[i_element];
	}
	return (*this);
}

SVector SVector::operator+(const SVector& rhs) const {
	SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		ret[i_element] = (*this)[i_element] + rhs[i_element];
	}
	return(ret);
}

SVector SVector::operator+(const int64_t rhs) const {
	SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		ret[i_element] = (*this)[i_element] + rhs;
	}
	return(ret);
}

SVector SVector::operator&(const SVector& rhs) const {
	SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		ret[i_element] = (*this)[i_element] & rhs[i_element];
	}
	return(ret);
}

SVector SVector::operator&(const int64_t rhs) const {
	SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		ret[i_element] = (*this)[i_element] & rhs;
	}
	return(ret);
}

SVector SVector::operator|(const SVector& rhs) const {
	SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		ret[i_element] = (*this)[i_element] | rhs[i_element];
	}
	return(ret);
}

SVector SVector::operator|(const int64_t rhs) const {
	SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		ret[i_element] = (*this)[i_element] | rhs;
	}
	return(ret);
}

SVector SVector::operator^(const SVector& rhs) const {
	SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		ret[i_element] = (*this)[i_element] ^ rhs[i_element];
	}
	return(ret);
}

SVector SVector::operator^(const int64_t rhs) const {
	SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		ret[i_element] = (*this)[i_element] ^ rhs;
	}
	return(ret);
}

SVector SVector::operator-(const SVector& rhs) const {
	SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		ret[i_element] = (*this)[i_element] - rhs[i_element];
	}
	return(ret);
}

SVector SVector::operator-(const int64_t rhs) const {
	SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		ret[i_element] = (*this)[i_element] - rhs;
	}
	return(ret);
}

SVector SVector::operator<<(const SVector& rhs) const {
	SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		ret[i_element] = (*this)[i_element] << rhs[i_element];
	}
	return(ret);
}

SVector SVector::operator<<(const uint64_t rhs) const {
	SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		ret[i_element] = (*this)[i_element] << rhs;
	}
	return(ret);
}

SVector SVector::operator>>(const SVector& rhs) const {
	SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		ret[i_element] = (*this)[i_element] >> rhs[i_element];
	}
	return(ret);
}

SVector SVector::operator>>(const uint64_t rhs) const {
	SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		ret[i_element] = (*this)[i_element] >> rhs;
	}
	return(ret);
}

SVector SVector::srl(const SVector& rhs) const {
	SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		ret[i_element] = (*this)[i_element].srl(rhs[i_element]);
	}
	return(ret);
}

SVector SVector::srl(const uint64_t rhs) const {
	SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		ret[i_element] = (*this)[i_element].srl(rhs);
	}
	return(ret);
}

SVRegister SVector::operator==(const SVector& rhs) const{
	SVRegister ret(length_*(*this)[0].width_in_bits_/8);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		if((*this)[i_element] == rhs[i_element]) ret.toggle_bit(i_element);
	}
	return(ret);
}

SVRegister SVector::operator==(const int64_t rhs) const{
	SVRegister ret(length_*(*this)[0].width_in_bits_/8);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		if((*this)[i_element] == rhs) ret.toggle_bit(i_element);
	}
	return(ret);
}

SVRegister SVector::operator!=(const SVector& rhs) const{
	SVRegister ret(length_*(*this)[0].width_in_bits_/8);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		if((*this)[i_element] != rhs[i_element]) ret.toggle_bit(i_element);
	}
	return(ret);
}

SVRegister SVector::operator!=(const int64_t rhs) const{
	SVRegister ret(length_*(*this)[0].width_in_bits_/8);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		if((*this)[i_element] != rhs) ret.toggle_bit(i_element);
	}
	return(ret);
}
// signed comparisons
SVRegister SVector::operator<(const SVector& rhs) const{
	SVRegister ret(length_*(*this)[0].width_in_bits_/8);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		if((*this)[i_element] < rhs[i_element]) ret.toggle_bit(i_element);
	}
	return(ret);
}
SVRegister SVector::operator<(const int64_t rhs) const{
	SVRegister ret(length_*(*this)[0].width_in_bits_/8);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		if((*this)[i_element] < rhs) ret.toggle_bit(i_element);
	}
	return(ret);
}
SVRegister SVector::operator<=(const SVector& rhs) const{
	SVRegister ret(length_*(*this)[0].width_in_bits_/8);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		if((*this)[i_element] <= rhs[i_element]) ret.toggle_bit(i_element);
	}
	return(ret);
}
SVRegister SVector::operator<=(const int64_t rhs) const{
	SVRegister ret(length_*(*this)[0].width_in_bits_/8);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		if((*this)[i_element] <= rhs) ret.toggle_bit(i_element);
	}
	return(ret);
}
SVRegister SVector::operator>(const SVector& rhs) const{
	return(rhs < *this);
}
SVRegister SVector::operator>(const int64_t rhs) const{
	SVRegister ret(length_*(*this)[0].width_in_bits_/8);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		if((*this)[i_element] > rhs) ret.toggle_bit(i_element);
	}
	return(ret);
}
SVRegister SVector::operator>=(const SVector& rhs) const{
	return(rhs <= *this);
}
SVRegister SVector::operator>=(const int64_t rhs) const{
	SVRegister ret(length_*(*this)[0].width_in_bits_/8);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		if((*this)[i_element] >= rhs) ret.toggle_bit(i_element);
	}
	return(ret);
}
// unsigned comparisons
SVRegister SVector::op_u_lt(const SVector& rhs) const{
	SVRegister ret(length_*(*this)[0].width_in_bits_/8);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		if( (*this)[i_element].op_u_lt(rhs[i_element])) ret.toggle_bit(i_element);
	}
	return(ret);
}
SVRegister SVector::op_u_lt(const uint64_t rhs) const{
	SVRegister ret(length_*(*this)[0].width_in_bits_/8);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		if( (*this)[i_element].op_u_lt(rhs)) ret.toggle_bit(i_element);
	}
	return(ret);
}
SVRegister SVector::op_u_lte(const SVector& rhs) const{
	SVRegister ret(length_*(*this)[0].width_in_bits_/8);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		if( (*this)[i_element].op_u_lte(rhs[i_element])) ret.toggle_bit(i_element);
	}
	return(ret);
}
SVRegister SVector::op_u_lte(const uint64_t rhs) const{
	SVRegister ret(length_*(*this)[0].width_in_bits_/8);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		if( (*this)[i_element].op_u_lte(rhs)) ret.toggle_bit(i_element);
	}
	return(ret);
}
SVRegister SVector::op_u_gt(const SVector& rhs) const{
	return ( rhs.op_u_lt(*this));
}
SVRegister SVector::op_u_gt(const uint64_t rhs) const{
	SVRegister ret(length_*(*this)[0].width_in_bits_/8);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		if( (*this)[i_element].op_u_gt(rhs)) ret.toggle_bit(i_element);
	}
	return(ret);
}
SVRegister SVector::op_u_gte(const SVector& rhs) const{
	return ( rhs.op_u_lte(*this));
}
SVRegister SVector::op_u_gte(const uint64_t rhs) const{
	SVRegister ret(length_*(*this)[0].width_in_bits_/8);
	for(size_t i_element = 0; i_element < length_; ++i_element){
		if( (*this)[i_element].op_u_gte(rhs)) ret.toggle_bit(i_element);
	}
	return(ret);
}

// masked ops
void SVector::m_assign(const SVector& vin, const SVRegister& vm, bool mask, size_t start_index){
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element] = vin[i_element];
	}
}

void SVector::m_assign(const int64_t rhs, const SVRegister& vm, bool mask, size_t start_index){
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element] = rhs;
	}
}

SVector& SVector::m_add(const SVector& opL, const SVector& rhs,  const SVRegister& vm, bool mask, size_t start_index) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_add(opL[i_element], rhs[i_element]);
	}
	return(*this);
}


SVector& SVector::m_add(const SVector& opL, const int64_t rhs, const SVRegister& vm, bool mask, size_t start_index ) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_add(opL[i_element], rhs);
	}
	return(*this);
}

SVector& SVector::m_sub(const SVector& opL, const SVector& rhs,  const SVRegister& vm, bool mask, size_t start_index) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_sub(opL[i_element], rhs[i_element]);
	}
	return(*this);
}

SVector& SVector::m_sub(const SVector& opL, const int64_t rhs, const SVRegister& vm, bool mask, size_t start_index ) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_sub(opL[i_element], rhs);
	}
	return(*this);
}

SVector& SVector::m_rsub(const int64_t lhs, const SVector& opR, const SVRegister& vm, bool mask, size_t start_index ) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_rsub(lhs, opR[i_element]);
	}
	return(*this);
}

SVector& SVector::m_waddu(const SVector& opL, const SVector& rhs,  const SVRegister& vm, bool mask, size_t start_index) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_waddu(opL[i_element], rhs[i_element]);
	}
	return(*this);
}

SVector& SVector::m_waddu(const SVector& opL, const int64_t rhs, const SVRegister& vm, bool mask, size_t start_index ) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_waddu(opL[i_element], rhs);
	}
	return(*this);
}

SVector& SVector::m_wsubu(const SVector& opL, const SVector& rhs,  const SVRegister& vm, bool mask, size_t start_index) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_wsubu(opL[i_element], rhs[i_element]);
	}
	return(*this);
}

SVector& SVector::m_wsubu(const SVector& opL, const int64_t rhs, const SVRegister& vm, bool mask, size_t start_index ) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_wsubu(opL[i_element], rhs);
	}
	return(*this);
}

SVector& SVector::m_wadd(const SVector& opL, const SVector& rhs,  const SVRegister& vm, bool mask, size_t start_index) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_wadd(opL[i_element], rhs[i_element]);
	}
	return(*this);
}

SVector& SVector::m_wadd(const SVector& opL, const int64_t rhs, const SVRegister& vm, bool mask, size_t start_index ) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_wadd(opL[i_element], rhs);
	}
	return(*this);
}

SVector& SVector::m_wsub(const SVector& opL, const SVector& rhs,  const SVRegister& vm, bool mask, size_t start_index) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_wsub(opL[i_element], rhs[i_element]);
	}
	return(*this);
}

SVector& SVector::m_wsub(const SVector& opL, const int64_t rhs, const SVRegister& vm, bool mask, size_t start_index ) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_wsub(opL[i_element], rhs);
	}
	return(*this);
}
//12.10 Multiplikation
SVector& SVector::m_ssmul(const SVector& opL, const SVector& rhs,  const SVRegister& vm, bool mask, size_t start_index) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_ssmul(opL[i_element], rhs[i_element]);
	}
	return(*this);
}
SVector& m_ssmul(const SVector& opL, const int64_t rhs, const SVRegister& vm, bool mask, size_t start_index){
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_ssmul(opL[i_element], rhs);
	}
	return(*this);
}



SVector& SVector::m_and(const SVector& opL, const SVector& rhs,  const SVRegister& vm, bool mask, size_t start_index) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_and(opL[i_element], rhs[i_element]);
	}
	return(*this);
}

SVector& SVector::m_and(const SVector& opL, const int64_t rhs, const SVRegister& vm, bool mask, size_t start_index ) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_and(opL[i_element], rhs);
	}
	return(*this);
}

SVector& SVector::m_or(const SVector& opL, const SVector& rhs,  const SVRegister& vm, bool mask, size_t start_index) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_or(opL[i_element], rhs[i_element]);
	}
	return(*this);
}

SVector& SVector::m_or(const SVector& opL, const int64_t rhs, const SVRegister& vm, bool mask, size_t start_index ) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_or(opL[i_element], rhs);
	}
	return(*this);
}

SVector& SVector::m_xor(const SVector& opL, const SVector& rhs,  const SVRegister& vm, bool mask, size_t start_index) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_xor(opL[i_element], rhs[i_element]);
	}
	return(*this);
}

SVector& SVector::m_xor(const SVector& opL, const int64_t rhs, const SVRegister& vm, bool mask, size_t start_index ) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_xor(opL[i_element], rhs);
	}
	return(*this);
}

SVector& SVector::m_sll(const SVector& opL, const SVector& rhs,  const SVRegister& vm, bool mask, size_t start_index) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_sll(opL[i_element], rhs[i_element]);
	}
	return(*this);
}

SVector& SVector::m_sll(const SVector& opL, const uint64_t rhs, const SVRegister& vm, bool mask, size_t start_index ) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_sll(opL[i_element], rhs);
	}
	return(*this);
}

SVector& SVector::m_sra(const SVector& opL, const SVector& rhs,  const SVRegister& vm, bool mask, size_t start_index) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_sra(opL[i_element], rhs[i_element]);
	}
	return(*this);
}

SVector& SVector::m_sra(const SVector& opL, const uint64_t rhs, const SVRegister& vm, bool mask, size_t start_index ) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_sra(opL[i_element], rhs);
	}
	return(*this);
}

SVector& SVector::m_srl(const SVector& opL, const SVector& rhs,  const SVRegister& vm, bool mask, size_t start_index) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_srl(opL[i_element], rhs[i_element]);
	}
	return(*this);
}

SVector& SVector::m_srl(const SVector& opL, const uint64_t rhs, const SVRegister& vm, bool mask, size_t start_index ) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element].s_srl(opL[i_element], rhs);
	}
	return(*this);
}

SVector& SVector::m_slideup(const SVector& opL, const uint64_t rhs, const SVRegister& vm, bool mask, size_t start_index ) {
	size_t max = rhs > start_index ? rhs : start_index;
	for(size_t i_element = max; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element))
			(*this)[i_element] = opL[i_element];
	}
	return(*this);
}

SVector& SVector::m_slidedown(const SVector& opL, const uint64_t rhs, const SVRegister& vm, bool mask, size_t vlmax, size_t start_index ) {
	for(size_t i_element = start_index; i_element < length_; ++i_element){
		if(!mask or vm.get_bit(i_element)) {
			size_t i_src_element = i_element + rhs;
			if(i_src_element < length_) {
				(*this)[i_element] = opL[i_element + rhs];
			} else {
				if (i_src_element < vlmax) {
					(*this)[i_element] = SVElement(opL[0].width_in_bits_ , opL[0].mem_ + i_src_element*opL[0].width_in_bits_/8);
				} else {
					(*this)[i_element] = 0;
				}
			}
		}
	}
	return(*this);
}
