package util

import (
	"crypto/rand"
	"math/big"
	"strings"
)

type PasswordUtilManger struct{}

func (PUM *PasswordUtilManger) FngetPassword(oldPassword string, passwordLength int) (string, error) {
	letters := "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	lowerLetters := "abcdefghijklmnopqrstuvwxy"
	numbers := "123456789"
	specialChars := "@/#$%&*"

	passwordLength -= 3

	var password strings.Builder

	for i := 0; i < passwordLength-2; i++ {
		char, err := PUM.FnrandomVal(letters)
		if err != nil {
			return "", err
		}
		password.WriteByte(char)
	}

	char, err := PUM.FnrandomVal(lowerLetters)
	if err != nil {
		return "", err
	}
	password.WriteByte(char)

	char, err = PUM.FnrandomVal(specialChars)
	if err != nil {
		return "", err
	}
	password.WriteByte(char)

	for i := 0; i < 2; i++ {
		char, err = PUM.FnrandomVal(numbers)
		if err != nil {
			return "", err
		}
		password.WriteByte(char)
	}

	shuffledPassword := PUM.FnshuffleString(password.String())

	if shuffledPassword == oldPassword {
		return PUM.FngetPassword(oldPassword, passwordLength+3) // Recursively generate a new password
	}

	return shuffledPassword, nil
}

func (PUM *PasswordUtilManger) FnrandomVal(charSet string) (byte, error) {
	max := big.NewInt(int64(len(charSet)))
	n, err := rand.Int(rand.Reader, max)
	if err != nil {
		return 0, err
	}
	return charSet[n.Int64()], nil
}

func (PUM *PasswordUtilManger) FnshuffleString(s string) string {
	r := []rune(s)
	for i := len(r) - 1; i > 0; i-- {
		j, _ := rand.Int(rand.Reader, big.NewInt(int64(i+1)))
		r[i], r[j.Int64()] = r[j.Int64()], r[i]
	}
	return string(r)
}

func (PUM *PasswordUtilManger) Fnencrypt(input string) string {
	var encrypted strings.Builder
	for i, c := range input {
		if i%2 == 0 {
			encrypted.WriteByte(byte(c) + 2)
		} else {
			encrypted.WriteByte(byte(c) + 1)
		}
	}
	return encrypted.String()
}

func (PUM *PasswordUtilManger) Fndecrypt(encrypted string) string {
	var decrypted strings.Builder
	for i, c := range encrypted {
		if i%2 == 0 {
			decrypted.WriteByte(byte(c) - 2)
		} else {
			decrypted.WriteByte(byte(c) - 1)
		}
	}
	return decrypted.String()
}
