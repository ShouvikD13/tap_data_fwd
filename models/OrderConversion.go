package models

import (
	"encoding/binary"
	"math"
)

type OrderConversionManager struct {
}

// Convert int16 to network (big-endian) byte order
func (OCM *OrderConversionManager) ConvertInt16ToNetworkOrder(i int16) int16 {
	b := make([]byte, 2)
	binary.BigEndian.PutUint16(b, uint16(i))
	return int16(binary.BigEndian.Uint16(b))
}

// Convert int16 from network (big-endian) byte order to host order
func (OCM *OrderConversionManager) ConvertInt16ToHostOrder(i int16) int16 {
	b := make([]byte, 2)
	binary.BigEndian.PutUint16(b, uint16(i))
	return int16(binary.BigEndian.Uint16(b))
}

// Convert int32 to network (big-endian) byte order
func (OCM *OrderConversionManager) ConvertInt32ToNetworkOrder(i int32) int32 {
	b := make([]byte, 4)
	binary.BigEndian.PutUint32(b, uint32(i))
	return int32(binary.BigEndian.Uint32(b))
}

// Convert int32 from network (big-endian) byte order to host order
func (OCM *OrderConversionManager) ConvertInt32ToHostOrder(i int32) int32 {
	b := make([]byte, 4)
	binary.BigEndian.PutUint32(b, uint32(i))
	return int32(binary.BigEndian.Uint32(b))
}

// Convert int64 to network (big-endian) byte order
func (OCM *OrderConversionManager) ConvertInt64ToNetworkOrder(i int64) int64 {
	b := make([]byte, 8)
	binary.BigEndian.PutUint64(b, uint64(i))
	return int64(binary.BigEndian.Uint64(b))
}

// Convert int64 from network (big-endian) byte order to host order
func (OCM *OrderConversionManager) ConvertInt64ToHostOrder(i int64) int64 {
	b := make([]byte, 8)
	binary.BigEndian.PutUint64(b, uint64(i))
	return int64(binary.BigEndian.Uint64(b))
}

// Convert float32 to network (big-endian) byte order
func (OCM *OrderConversionManager) ConvertFloat32ToNetworkOrder(f float32) float32 {
	bits := math.Float32bits(f)
	networkBits := OCM.ConvertInt32ToNetworkOrder(int32(bits))
	return math.Float32frombits(uint32(networkBits))
}

// Convert float32 from network (big-endian) byte order to host order
func (OCM *OrderConversionManager) ConvertFloat32ToHostOrder(f float32) float32 {
	bits := math.Float32bits(f)
	hostBits := OCM.ConvertInt32ToHostOrder(int32(bits))
	return math.Float32frombits(uint32(hostBits))
}

// Convert float64 to network (big-endian) byte order
func (OCM *OrderConversionManager) ConvertFloat64ToNetworkOrder(f float64) float64 {
	bits := math.Float64bits(f)
	networkBits := OCM.ConvertInt64ToNetworkOrder(int64(bits))
	return math.Float64frombits(uint64(networkBits))
}

// Convert float64 from network (big-endian) byte order to host order
func (OCM *OrderConversionManager) ConvertFloat64ToHostOrder(f float64) float64 {
	bits := math.Float64bits(f)
	hostBits := OCM.ConvertInt64ToHostOrder(int64(bits))
	return math.Float64frombits(uint64(hostBits))
}
