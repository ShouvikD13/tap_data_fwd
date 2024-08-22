package models

import (
	"encoding/binary"
	"math"
)

func convertInt16ToNetworkOrder(i int16) int16 {
	return int16(binary.BigEndian.Uint16([]byte{byte(i >> 8), byte(i)}))
}

func convertInt16ToHostOrder(i int16) int16 {
	b := []byte{byte(i >> 8), byte(i)}
	return int16(binary.BigEndian.Uint16(b))
}

func convertInt32ToNetworkOrder(i int32) int32 {
	return int32(binary.BigEndian.Uint32([]byte{
		byte(i >> 24), byte(i >> 16), byte(i >> 8), byte(i),
	}))
}

func convertInt32ToHostOrder(i int32) int32 {
	b := []byte{byte(i >> 24), byte(i >> 16), byte(i >> 8), byte(i)}
	return int32(binary.BigEndian.Uint32(b))
}

func convertFloat32ToNetworkOrder(f float32) float32 {
	bits := math.Float32bits(f)
	bits = convertInt32ToNetworkOrder(int32(bits))
	return math.Float32frombits(uint32(bits))
}

func convertFloat32ToHostOrder(f float32) float32 {
	bits := math.Float32bits(f)
	bits = convertInt32ToHostOrder(int32(bits))
	return math.Float32frombits(uint32(bits))
}

func convertFloat64ToNetworkOrder(f float64) float64 {
	bits := math.Float64bits(f)
	bits = convertInt64ToNetworkOrder(int64(bits))
	return math.Float64frombits(uint64(bits))
}

func convertFloat64ToHostOrder(f float64) float64 {
	bits := math.Float64bits(f)
	bits = convertInt64ToHostOrder(int64(bits))
	return math.Float64frombits(uint64(bits))
}

func convertFloat64ToNetworkOrder(f float64) float64 {
	bits := math.Float64bits(f)
	bits = convertInt64ToNetworkOrder(int64(bits))
	return math.Float64frombits(uint64(bits))
}

func convertFloat64ToHostOrder(f float64) float64 {
	bits := math.Float64bits(f)
	bits = convertInt64ToHostOrder(int64(bits))
	return math.Float64frombits(uint64(bits))
}
