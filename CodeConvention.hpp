#ifndef EXAMPLE_CODECONVENTION_HPP
#define EXAMPLE_CODECONVENTION_HPP

#ifndef SOME_DEFINITION
    #include <cstdint>
#endif
#include <string>

namespace ggj
{
    constexpr int ANOTHER_CONST = 32;  

    /*!
     * Adds two integers together. This is an example to showcase doxygen documentation.
     * Static functions are Pascal Case, non-static functions are camelCase. 
     *
     * @param lhs The first number
     * @param rhs The second number
     * @return The result of the two numbers added together
     */
    static constexpr int AddNumbers(int lhs, int rhs)
    {
        return lhs + rhs;
    }

    /*!
     * A small struct showing the field, one-liner, and defaulted-operator style.
     */
    struct StructExample
    {
        bool isSolid { false };
        uint32_t someInt { 0 };

        //! OK for simple one liners to be on same line.
        [[nodiscard]] constexpr bool isEmpty() const
        {
            return !isSolid;
        }

        [[nodiscard]] constexpr std::string doDaSwitch(int daNumber) const
        {
            switch (daNumber)
            {
                case 0:
                    return "Zero";
                case 1:
                    return "One";
                default:
                    return "Other";
            }
        }

        constexpr bool operator==(const StructExample&) const = default;

    };

    /*!
     * Interface example: `I`-prefixed, pure-virtual, with a virtual destructor.
     */
    class IClassExample
    {
        public:
            virtual ~IClassExample() = default;

            /*!
             * @return The width, in arbitrary units.
             */
            [[nodiscard]] virtual int width() const = 0;

            /*!
             * @return The height, in arbitrary units.
             */
            [[nodiscard]] virtual int height() const = 0;
    };

    /*!
     * Concrete example deriving from the interface; `final` since it is not meant
     * to be subclassed further.
     */
    class ClassExample final : public IClassExample
    {
        public:
            [[nodiscard]] int width() const override { return m_width; }

            [[nodiscard]] int height() const override { return m_height; }

            static constexpr int MY_CONST_INT = 16;
            int somePublicInt { 0 }; ///< Publicly exposed members.

        private:
            int m_width { 0 };  ///< "m_"-prefix for internal members: protected or private.
            int m_height { 0 }; ///< Same convention for every non-public field.
    };
} // namespace ggj

#endif // EXAMPLE_CODECONVENTION_HPP
