#pragma once

class Color_D3D
{
public:

	Color_D3D() : m_packedColor(0) {}
	~Color_D3D() {}

	inline Color_D3D(const Color &other);
	inline Color_D3D(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
	inline Color_D3D& operator=(Color &other);
	inline void setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

private:

	unsigned int m_packedColor;
};

inline Color_D3D::Color_D3D(const Color &other)
{
	*this = other;
}

inline Color_D3D::Color_D3D(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	setColor(r, g, b, a);
}

inline Color_D3D& Color_D3D::operator=(Color &other)
{
	unsigned char r = static_cast<unsigned char>(other.r * 255u);
	unsigned char g = static_cast<unsigned char>(other.g * 255u);
	unsigned char b = static_cast<unsigned char>(other.b * 255u);
	unsigned char a = static_cast<unsigned char>(other.a * 255u);
	setColor(r, g, b, a);

	return *this;
}

inline void Color_D3D::setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	// D3D ABGR packing
	m_packedColor = (a << 24) | (b << 16) | (g << 8) | r;
}
