#pragma once

namespace fast_io
{

namespace details
{

template<std::size_t bs,bool uppercase=false>
struct base_number_upper_constraints
{
	explicit base_number_upper_constraints() = default;
	static constexpr bool value = 2<=bs&&bs<=36&&((bs<=10&&!uppercase)||10<bs);
};

template<char8_t base,bool uppercase,bool point=false,std::random_access_iterator Iter,typename U>
requires (!std::signed_integral<U>)
inline constexpr auto output_base_number_impl(Iter iter,U a)
{
//number: 0:48 9:57
//upper: 65 :A 70: F
//lower: 97 :a 102 :f
	constexpr auto &table(details::shared_static_base_table<base,uppercase>::table);
	constexpr std::uint32_t pw(static_cast<std::uint32_t>(table.size()));
	constexpr std::size_t chars(table.front().size());
	for(;pw<=a;)
	{
		auto const rem(a%pw);
		a/=pw;
		my_copy_n(table[rem].data(),chars,iter-=chars);
	}
	if constexpr(chars==2)
	{
		if(base<=a)
		{
			auto const& tm(table[a]);
			
			if constexpr(point)
			{
				*--iter=tm[1];
				*--iter=0x2E;
				*--iter=tm.front();
			}
			else
			{
				my_copy_n(tm.data(),chars,iter-=chars);
			}
		}
		else
		{
			if constexpr(point)
				*--iter=0x2E;
			if constexpr(10 < base)
			{
				if(a<10)
					*--iter = a+0x30;
				else
				{
					if constexpr (uppercase)
						*--iter = a+55;	
					else
						*--iter = a+87;
				}
			}
			else
				*--iter=a+0x30;
		}
	}
	else
	{
		if(base<=a)
		{
			auto const& tm(table[a]);
			auto i(tm.data());
			for(;*i==0x30;++i);
			auto const ed(tm.data()+chars);
			if constexpr(point)
			{
				my_copy(i+1,ed,iter-=ed-(i+1));
				*--iter=0x2E;
				*--iter=*i;
			}
			else
				my_copy(i,ed,iter-=ed-i);
		}
		else
		{
			if constexpr(point)
				*--iter=0x2E;
			if constexpr(10 < base)
			{
				if(a<10)
					*--iter = a+0x30;
				else
				{
					if constexpr (uppercase)
						*--iter = a+55;	
					else
						*--iter = a+87;
				}
			}
			else
				*--iter=a+0x30;
		}
	}
	return iter;
}

template<std::uint32_t base,bool ryu_mode=false,typename U>
requires (!std::signed_integral<U>)
inline constexpr std::size_t chars_len(U value) noexcept
{
	if constexpr(base==10&&sizeof(U)<9)
	{
		if constexpr(7<sizeof(U))
		{
			if constexpr(!ryu_mode)
			{
				if(10000000000000000000ULL<=value)
					return 20;
				if(1000000000000000000ULL<=value)
					return 19;
				if(100000000000000000ULL<=value)
					return 18;
			}
			if(10000000000000000ULL<=value)
				return 17;
			if(1000000000000000ULL<=value)
				return 16;
			if(100000000000000ULL<=value)
				return 15;
			if(10000000000000ULL<=value)
				return 14;
			if(1000000000000ULL<=value)
				return 13;
			if(100000000000ULL<=value)
				return 12;
			if(10000000000ULL<=value)
				return 11;
		}
		if constexpr(3<sizeof(U))
		{
			if constexpr(4<sizeof(U)||!ryu_mode)
			{
				if(1000000000U<=value)
					return 10;
			}
			if(100000000U<=value)
				return 9;
			if(10000000U<=value)
				return 8;
			if(1000000U<=value)
				return 7;
			if(100000U<=value)
				return 6;
		}
		if constexpr(1<sizeof(U))
		{
			if(10000U<=value)
				return 5;
			if(1000U<=value)
				return 4;
		}
		if(100U<=value)
			return 3;
		if(10U<=value)
			return 2;
		return 1;
	}
	else
	{
		constexpr std::uint32_t base2(base  * base);
		constexpr std::uint32_t base3(base2 * base);
		constexpr std::uint32_t base4(base3 * base);
		for (std::size_t n(1);;n+=4)
		{
			if (value < base)
				return n;
			if (value < base2)
				return n + 1;
			if (value < base3)
				return n + 2;
			if (value < base4)
				return n + 3;
			value /= base4;
		}
	}
}


template<char8_t base,bool uppercase,bool ln=false,output_stream output,std::unsigned_integral U>
inline constexpr void output_base_number(output& out,U a)
{
	if constexpr(buffer_output_stream<output>)
	{
		if constexpr(ln)
		{
			auto reserved(oreserve(out,chars_len<base,false>(a)+1));
			if constexpr(std::is_pointer_v<decltype(reserved)>)
			{
				if(reserved)
				{
					*--reserved=u8'\n';
					output_base_number_impl<base,uppercase>(reserved,a);
					return;
				}
			}
			else
			{
				*--reserved=u8'\n';
				output_base_number_impl<base,uppercase>(std::to_address(reserved),a);
				return;
			}
		}
		else
		{
			auto reserved(oreserve(out,chars_len<base,false>(a)));
			if constexpr(std::is_pointer_v<decltype(reserved)>)
			{
				if(reserved)
				{
					output_base_number_impl<base,uppercase>(reserved,a);
					return;
				}
			}
			else
			{
				output_base_number_impl<base,uppercase>(std::to_address(reserved),a);
				return;
			}
		}
	}
	if constexpr(ln)
	{
		std::array<typename output::char_type,sizeof(a)*8+1> v;
		v.back()=0xA;
		auto const e(v.data()+v.size());
		write(out,output_base_number_impl<base,uppercase>(e-1,a),e);
	}
	else
	{
		std::array<typename output::char_type,sizeof(a)*8> v;
		auto const e(v.data()+v.size());
		write(out,output_base_number_impl<base,uppercase>(e,a),e);
	}
}

template<char8_t base,bool uppercase,bool ln=false,output_stream output,std::signed_integral T>
inline constexpr void output_base_number(output& out,T b)
{
	bool const minus(b<0);
	auto const a(static_cast<std::make_unsigned_t<T>>(minus?-b:b));
	if constexpr(buffer_output_stream<output>)
	{
		if constexpr(ln)
		{
			auto reserved(oreserve(out,chars_len<base>(a)+1+minus));
			if constexpr(std::is_pointer_v<decltype(reserved)>)
			{
				if(reserved)
				{
					*--reserved=0xA;
					auto p(output_base_number_impl<base,uppercase>(reserved,a));
					if(minus)
						*--p=0x2d;
					return;
				}
			}
			else
			{
				*--reserved=0xA;
				auto p(output_base_number_impl<base,uppercase>(reserved,a));
				if(minus)
					*--p=0x2d;
				return;
			}
		}
		else
		{
			auto reserved(oreserve(out,chars_len<base>(a)+minus));
			if constexpr(std::is_pointer_v<decltype(reserved)>)
			{
				if(reserved)
				{
					auto p(output_base_number_impl<base,uppercase>(reserved,a));
					if(minus)
						*--p=0x2d;
					return;
				}
			}
			else
			{
				auto p(output_base_number_impl<base,uppercase>(reserved,a));
				if(minus)
					*--p=0x2d;
				return;
			}
		}
	}
	if constexpr(ln)
	{
		std::array<typename output::char_type,sizeof(a)*8+2> v;
		v.back()=0xA;
		auto const e(v.data()+v.size());
		auto iter(output_base_number_impl<base,uppercase>(e-1,a));
		if(minus)
			*--iter=0x2d;
		write(out,iter,e);		
	}
	else
	{
		std::array<typename output::char_type,sizeof(a)*8+1> v;
		auto const e(v.data()+v.size());
		auto iter(output_base_number_impl<base,uppercase>(e,a));
		if(minus)
			*--iter=0x2d;
		write(out,iter,e);
	}
}

template<bool sign,std::uint8_t base>
requires (0x2<base&&base<=0x36)
struct is_numerical
{
template<std::integral T>
inline constexpr bool operator()(T ch)
{
	std::make_unsigned_t<T> e(ch);
	if constexpr(sign)
	{
		if constexpr(base<=0xA)
			return (e==0x2d)||static_cast<std::uint8_t>(e-0x30)<base;
		else
		{
			constexpr std::uint8_t basem10(base-0xa);
			return (e==0x2d)||static_cast<std::uint8_t>((e-0x30)<0xA)||
				(static_cast<std::uint8_t>(e-0x41)<basem10)||
				(static_cast<std::uint8_t>(e-0x61)<basem10);
		}
	}
	else
	{
		if constexpr(base<=0xA)
			return static_cast<std::uint8_t>(e-0x30)<base;
		else
		{
			constexpr std::uint8_t basem10(base-0xa);
			return (static_cast<std::uint8_t>(e-0x30)<0xA)||
				(static_cast<std::uint8_t>(e-0x41)<basem10)||
				(static_cast<std::uint8_t>(e-0x61)<basem10);
		}
	}
}
};

template<std::integral T,std::integral T1>
inline constexpr T mul_overflow(T a,T1 b)
{
	T t{};
	if(__builtin_mul_overflow(a,b,std::addressof(t)))
#ifdef __cpp_exceptions
		throw std::overflow_error("mul overflow");
#else
		fast_terminate();
#endif
	return t;
}

template<std::integral T,std::integral T1>
inline constexpr T add_overflow(T a,T1 b)
{
	T t{};
	if(__builtin_add_overflow(a,b,std::addressof(t)))
#ifdef __cpp_exceptions
		throw std::overflow_error("add overflow");
#else
		fast_terminate();
#endif
	return t;
}

template<std::integral T>
inline constexpr bool is_space(T const u)
{
	if constexpr(std::unsigned_integral<std::remove_cvref_t<T>>)
	{
		return u==0x20||(u-0x9)<0x4;
	}
	else
	{
		std::make_unsigned_t<T> const e(u);
		return e==0x20||(e-0x9)<0x4;
	}
}

template<std::integral T,char8_t base,buffer_input_stream input>
inline constexpr T input_base_number(input& in)
{
	using unsigned_char_type = std::make_unsigned_t<typename input::char_type>;
	using unsigned_t = std::make_unsigned_t<T>;
	if constexpr(std::unsigned_integral<T>)
	{
		unsigned_char_type fr(front(in));
		if(fr==0x2b)[[unlikely]]
			fr=next_unsigned(in);
		if constexpr(base<=10)
		{
			if(static_cast<unsigned_char_type>(base)<=static_cast<unsigned_char_type>(fr-=0x30))
			#ifdef __cpp_exceptions
				throw std::runtime_error("illegal input");
			#else
				fast_terminate();
			#endif
			T t(fr);
			for(;;)
			{
				auto f(next_unsigned<2>(in));
				if(static_cast<unsigned_char_type>(base)<=static_cast<unsigned_char_type>(f-=0x30))
					break;
				t=t*base+f;
			}
			return t;
		}
		else
		{
			constexpr unsigned_char_type bm10(base-10);
			T t{};
			if(static_cast<unsigned_char_type>(fr-=0x30)<static_cast<unsigned_char_type>(10))
				t=fr;
			else if(static_cast<unsigned_char_type>(fr-=17)<bm10||static_cast<unsigned_char_type>(fr-=32)<bm10)
				t=fr+10;
			else[[unlikely]]
			#ifdef __cpp_exceptions
				throw std::runtime_error("illegal input");
			#else
				fast_terminate();
			#endif
			for(;;)
			{
				auto f(next_unsigned<2>(in));
				if(static_cast<unsigned_char_type>(f-=0x30)<static_cast<unsigned_char_type>(10))
					t=t*base+f;
				else if(static_cast<unsigned_char_type>(f-=17)<bm10||static_cast<unsigned_char_type>(f-=32)<bm10)
					t=t*base+(f+10);
				else[[unlikely]]
					break;
			}
			return t;	
		}
	}
	else
	{
		unsigned_char_type fr(front(in));
		bool const sign(fr==0x2d);
		if(sign||fr==0x2b)
			fr=next_unsigned(in);
		if constexpr(base<=10)
		{
			if(static_cast<unsigned_char_type>(base)<=static_cast<unsigned_char_type>(fr-=0x30))
			#ifdef __cpp_exceptions
				throw std::runtime_error("illegal input");
			#else
				fast_terminate();
			#endif
			unsigned_t t(fr);
			for(;;)
			{
				auto f(next_unsigned<2>(in));
				if(static_cast<unsigned_char_type>(base)<=static_cast<unsigned_char_type>(f-=0x30))
					break;
				t=t*base+f;
			}
			if(static_cast<unsigned_t>(std::numeric_limits<T>::max())+static_cast<unsigned_t>(sign)<t)
			#ifdef __cpp_exceptions
				throw std::overflow_error("signed overflow");
			#else
				fast_terminate();
			#endif
			if(sign)
			{
				if(t==0x1+static_cast<unsigned_t>(std::numeric_limits<T>::max()))
					return std::numeric_limits<T>::min();
				return -static_cast<T>(t);
			}
			return t;
		}
		else
		{
			constexpr unsigned_char_type bm10(base-10);
			unsigned_t t{};
			if(static_cast<unsigned_char_type>(fr-=0x30)<static_cast<unsigned_char_type>(10))
				t=fr;
			else if(static_cast<unsigned_char_type>(fr-=17)<bm10||static_cast<unsigned_char_type>(fr-=32)<bm10)
				t=fr+10;
			else[[unlikely]]
			#ifdef __cpp_exceptions
				throw std::runtime_error("illegal input");
			#else
				fast_terminate();
			#endif
			for(;;)
			{
				auto f(next_unsigned<2>(in));
				if(static_cast<unsigned_char_type>(f-=0x30)<static_cast<unsigned_char_type>(10))
					t=add_overflow(mul_overflow(t,base),f);
				else if(static_cast<unsigned_char_type>(f-=17)<bm10||static_cast<unsigned_char_type>(f-=32)<bm10)
					t=add_overflow(mul_overflow(t,base),f+10);
				else[[unlikely]]
					break;
			}
			if(static_cast<unsigned_t>(std::numeric_limits<T>::max())+static_cast<unsigned_t>(sign)<t)
			#ifdef __cpp_exceptions
				throw std::overflow_error("signed overflow");
			#else
				fast_terminate();
			#endif
			if(sign)
			{
				if(t==0x1+static_cast<unsigned_t>(std::numeric_limits<T>::max()))
					return std::numeric_limits<T>::min();
				return -static_cast<T>(t);
			}
			return t;
		}
	}
}

}

namespace manip
{
	
template<std::size_t bs,bool uppercase,typename T>
requires fast_io::details::base_number_upper_constraints<bs,uppercase>::value
struct base_t
{
	T& reference;
};

template<std::size_t bs,bool uppercase,typename T,std::integral char_type>
requires fast_io::details::base_number_upper_constraints<bs,uppercase>::value
struct base_split_t
{
	T& reference;
	char_type character;
};

}

template<std::size_t b,bool uppercase=false,typename T>
inline constexpr manip::base_t<b,uppercase,T> base(T& t) {return {t};}
template<std::size_t b,bool uppercase=false,typename T>
inline constexpr manip::base_t<b,uppercase,T const> base(T const& t) {return {t};}

template<typename T> inline constexpr manip::base_t<16,false,T> hex(T& t) {return {t};}
template<typename T> inline constexpr manip::base_t<16,false,T const> hex(T const& t){return {t};}

template<typename T> inline constexpr manip::base_t<16,true,T> hexupper(T& t){return {t};}
template<typename T> inline constexpr manip::base_t<16,true,T const> hexupper(T const& t) {return {t};}

template<typename T> inline constexpr manip::base_t<8,false,T> oct(T& t) {return {t};}
template<typename T> inline constexpr manip::base_t<8,false,T const> oct(T const& t){return {t};}

template<typename T> inline constexpr manip::base_t<10,false,T> dec(T& t) {return {t};}
template<typename T> inline constexpr manip::base_t<10,false,T const> dec(T const& t){return {t};}

template<typename T> inline constexpr manip::base_t<2,false,T> bin(T& t){return {t};}
template<typename T> inline constexpr manip::base_t<2,false,T const> bin(T const& t) {return {t};}


template<std::size_t b,bool uppercase=false,typename T,std::integral char_type>
inline constexpr manip::base_split_t<b,uppercase,T,char_type> base_split(T& t,char_type ch) {return {t,ch};}
template<std::size_t b,bool uppercase=false,typename T,std::integral char_type>
inline constexpr manip::base_split_t<b,uppercase,T const,char_type> base_split(T const& t,char_type ch) {return {t,ch};}

template<typename T,std::integral char_type> inline constexpr manip::base_split_t<16,false,T,char_type> hex_split(T& t,char_type ch) {return {t,ch};}
template<typename T,std::integral char_type> inline constexpr manip::base_split_t<16,false,T const,char_type> hex_split(T const& t,char_type ch){return {t,ch};}

template<typename T,std::integral char_type> inline constexpr manip::base_split_t<16,true,T,char_type> hexupper_split(T& t,char_type ch){return {t,ch};}
template<typename T,std::integral char_type> inline constexpr manip::base_split_t<16,true,T const,char_type> hexupper_split(T const& t,char_type ch) {return {t,ch};}

template<typename T,std::integral char_type> inline constexpr manip::base_split_t<8,false,T,char_type> oct_split(T& t,char_type ch) {return {t,ch};}
template<typename T,std::integral char_type> inline constexpr manip::base_split_t<8,false,T const,char_type> oct_split(T const& t,char_type ch){return {t,ch};}

template<typename T,std::integral char_type> inline constexpr manip::base_split_t<10,false,T,char_type> dec_split(T& t,char_type ch) {return {t,ch};}
template<typename T,std::integral char_type> inline constexpr manip::base_split_t<10,false,T const,char_type> dec_split(T const& t,char_type ch){return {t,ch};}

template<typename T,std::integral char_type> inline constexpr manip::base_split_t<2,false,T,char_type> bin_split(T& t,char_type ch){return {t,ch};}
template<typename T,std::integral char_type> inline constexpr manip::base_split_t<2,false,T const,char_type> bin_split(T const& t,char_type ch) {return {t,ch};}

template<std::size_t base,bool uppercase,character_output_stream output,std::integral T>
inline constexpr void print_define(output& out,manip::base_t<base,uppercase,T> v)
{
	details::output_base_number<base,uppercase>(out,v.reference);
}

template<std::size_t base,bool uppercase,character_output_stream output,std::integral T>
inline constexpr void println_define(output& out,manip::base_t<base,uppercase,T> v)
{
	details::output_base_number<base,uppercase,true>(out,v.reference);
}

template<std::size_t base,bool uppercase,buffer_input_stream input,std::integral T>
inline constexpr bool scan_define(input& in,manip::base_t<base,uppercase,T> v)
{
	if(!skip_space(in))
		return false;
	v.reference=details::input_base_number<std::remove_cvref_t<T>,base>(in);
	return true;
}


template<buffer_input_stream input,std::integral T>
inline constexpr bool scan_define(input& in,T& a)
{
	if(!skip_space(in))
		return false;
	a=details::input_base_number<std::remove_cvref_t<T>,10>(in);
	return true;
}

template<output_stream output,std::integral T>
inline constexpr void print_define(output& out,T a)
{
	if constexpr(std::unsigned_integral<T>)
		details::jiaendu::output<false>(out,a);
	else if constexpr(std::signed_integral<T>)
	{
		using unsigned_t = std::make_unsigned_t<T>;
		if(a<0)
			details::jiaendu::output<false,true>(out,static_cast<unsigned_t>(-a));
		else
			details::jiaendu::output<false>(out,static_cast<unsigned_t>(a));
	}
}

template<output_stream output,std::integral T>
inline constexpr void println_define(output& out,T a)
{
	if constexpr(std::unsigned_integral<T>)
		details::jiaendu::output<true>(out,a);
	else if constexpr(std::signed_integral<T>)
	{
		using unsigned_t = std::make_unsigned_t<T>;
		if(a<0)
			details::jiaendu::output<true,true>(out,static_cast<unsigned_t>(-a));
		else
			details::jiaendu::output<true>(out,static_cast<unsigned_t>(a));
	}
}

template<std::size_t base,bool uppercase,output_stream output,typename T>
requires std::same_as<std::byte,std::remove_cvref_t<T>>
inline constexpr void print_define(output& out,manip::base_t<base,uppercase,T> v)
{
	details::output_base_number<base,uppercase>(out,std::to_integer<char8_t>(v.reference));
}

template<std::size_t base,bool uppercase,buffer_input_stream input,typename T>
requires std::same_as<std::byte,std::remove_cvref_t<T>>
inline constexpr bool scan_define(input& in,manip::base_t<base,uppercase,T> v)
{
	if(!skip_space(in))
		return false;
	v.reference=static_cast<std::byte>(details::input_base_number<char8_t,base>(in));
	return true;
}


template<buffer_input_stream input,typename T>
requires std::same_as<std::byte,std::remove_cvref_t<T>>
inline constexpr bool scan_define(input& in,T& a)
{
	if(!skip_space(in))
		return false;
	a=static_cast<std::byte>(details::input_base_number<char8_t,10>(in));
	return true;
}

template<output_stream output,typename T>
requires std::same_as<std::byte,std::remove_cvref_t<T>>
inline constexpr void print_define(output& out,T& a)
{
	details::output_base_number<10,false>(out,std::to_integer<char8_t>(a));
}

template<output_stream output,typename T>
requires std::same_as<std::byte,std::remove_cvref_t<T>>
inline constexpr void println_define(output& out,T& a)
{
	details::output_base_number<10,false,true>(out,std::to_integer<char8_t>(a));
}


template<std::size_t bas,bool uppercase,std::integral char_type,character_output_stream output,std::ranges::range T>
inline constexpr void print_define(output& out,manip::base_split_t<bas,uppercase,T,char_type> rangeref)
{
	auto i(cbegin(rangeref.reference));
	auto e(cend(rangeref.reference));
	if(i==e)
		return;
	print(out,base<bas,uppercase>(*i));
	for(++i;i!=e;++i)
		print(out,char_view(rangeref.character),base<bas,uppercase>(*i));
}

}
