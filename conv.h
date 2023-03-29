/**
 * conv.h
 * 
 * the MicroChess project: https://github.com/ripred/MicroChess
 * 
 * header file for MicroChess
 * 
 */
#ifndef CONV_INCL
#define CONV_INCL

/**
 * @brief conv1_t struct represents a single chess piece location in 8 bits.
 * 
 * The struct cleverly packs the data for a chess piece location into 2 bytes. It 
 * uses a union of two structures to allow for easy access to the data in two 
 * different formats. The first structure, pt, is a bitfield that is laid out as 
 * † follows:
 * 
 *   0   1   2   3   4   5   6   7   8   9
 * +---+---+---+---+---+---+---+---+---+------+
 * |    col    |    row    |   type    | side |
 * +---+---+---+---+---+---+---+---+---+------+
 * 
 * The second structure, ndx, is laid out as follows:
 * 
 *   0   1   2   3   4   5   6   7   8   9
 * +---+---+---+---+---+---+---+---+---+------+
 * |          index        |   type    | side |
 * +---+---+---+---+---+---+---+---+---+------+
 * 
 * The idea behind the design is that the col and row fields are easily accessed
 * when scanning the chess board in row-major order, while the index field is
 * easily accessed when looping over an array of pieces. Additionally, assigning
 * to the col and row fields automatically performs the calculations
 * `index = col + row * 8` and `col = ndx % 8, row = index / 8` implicitly, thanks 
 * to the clever use of the union and the resulting alignment of the bitfields!
 * 
 * Let me say that again: The math and conversion between the col/row and the
 * 0 - 63 and the (col,row) pairs automatically happens without having to apply
 * the `index = col + row * 8`, `col = index % 8`, or the `row = index / 8`
 * expressions! To understand this realize that dividing by 8 or multiplying by
 * 8 is the same operation as shifting a value 3 bits up or down. By aligning
 * the bitfields the way we are, we are forcing the 3 bits that store the row
 * to implicitly be 3 bits to the left of the col bitfield when viewed from
 * perspective of the ndx.index field! Binary numbers are cool!
 * 
 * The struct provides a set of getter and setter methods to safely access the
 * bit fields, as well as constructors to initialize the values of the fields.
 * 
 * Forcing all access to go through the the setters and getters is not just there 
 * to ensure that the fields cannot be accessed except through the methods; they 
 * serve a higher purpose. All bitfields should be able to be accessed in two
 * bitwise assembly instructions: A shift operation (<< or >>) and a masking AND 
 * instruction. By providing these accessors we guarantee that all code produced by 
 * the compiler will use two assembly instructions per access worst-case when 
 * accessing any of these fields. Without this it is possible for the compiler to 
 * generate some inefficient code such as loading the entire structure into memory 
 * just to perform an operation on one of the fields and then throwing the temporary 
 * stack object away.
 * 
 * † The actual layout wrt the order of the fields in memory is up to the compiler 
 * based on the processor architecture. The intent and optimial storage is still 
 * achieved however regardless of the order in physical memory.
 * 
 */
struct conv1_t {
private:

    union {
        struct {
            uint8_t     col : 3,    ///< The column value of the position.
                        row : 3,    ///< The row value of the position.
                       type : 3,    ///< The type of piece (pawn, knight, etc.) at the position.
                       side : 1;    ///< The side (white or black) of the piece at the position.
        } pt;                       ///< A struct used to compactly store the values and to access 
                                    ///  the member fields of the union in a type-safe manner.
        struct {
            uint8_t   index : 6,    ///< The index of the position in the board array.
                       type : 3,    ///< The type of piece (pawn, knight, etc.) at the position.
                       side : 1;    ///< The side (white or black) of the piece at the position.
        } ndx;                      ///< A struct used to compactly store the values and to access 
                                    ///  the member fields of the union in a type-safe manner.
    } u;                            ///< A union used to store the position information in two 
                                    ///  different ways, for different use cases.
public:

    /**
    * @brief Default constructor. Initializes the piece to an empty black piece at (0, 0).
    */
    conv1_t() : u{ 0, 0, Empty, Black } {}

    /**
    * @brief Constructor that takes an index and initializes the piece to the corresponding row and column.
    * 
    * @param index The index of the piece, where 0 represents the top-left square on the board.
    */
    conv1_t(uint8_t index) {
        u.ndx.index = index;
        u.ndx.type = Empty;
        u.ndx.side = Black;
    }

    /**
    * @brief Constructor that takes an index, type, and side, and initializes the piece to those values.
    * 
    * @param index The index of the piece, where 0 represents the top-left square on the board.
    * @param type The type of the piece.
    * @param side The side of the piece.
    */
    conv1_t(uint8_t index, uint8_t type, uint8_t side) {
        u.ndx.index = index;
        u.ndx.type = type;
        u.ndx.side = side;
    }

    /**
    * @brief Constructor that takes a column and row, and initializes the piece to those values.
    * 
    * @param col The column of the piece, where 0 represents the leftmost column on the board.
    * @param row The row of the piece, where 0 represents the top row on the board.
    */
    conv1_t(uint8_t col, uint8_t row) : u{ col, row, Empty, Black } {}

    /**
    * @brief Set the index field of the ndx structure.
    * 
    * @param value The value to set the index field to.
    */
    void set_index(uint8_t value) { u.ndx.index = value; }

    /**
    * @brief Set the col field of the pt structure.
    * 
    * @param value The value to set the col field to.
    */
    void set_col(uint8_t value) { u.pt.col = value; }

    /**
    * @brief Set the row field of the pt structure.
    * 
    * @param value The value to set the row field to.
    */
    void set_row(uint8_t value) { u.pt.row = value; }

    /**
    * @brief Set the type field of the pt or ndx structure.
    * 
    * @param value The value to set the type field to.
    */
    void set_type(uint8_t value) { u.pt.type = u.ndx.type = value; }

    /**
    * @brief Set the side field of the pt or ndx structure.
    * 
    * @param value The value to set the side field to.
    */
    void set_side(uint8_t value) { u.pt.side = u.ndx.side = value; }

    /**
    * @brief Getter for the `index` field of the `ndx` structure.
    * 
    * @return uint8_t The value of the `index` field.
    */
    uint8_t get_index() const { return u.ndx.index; }

    /**
    * @brief Getter for the `col` field of the `pt` structure.
    * 
    * @return uint8_t The value of the `col` field.
    */
    uint8_t get_col() const { return u.pt.col; }

    /**
    * @brief Getter for the `row` field of the `pt` structure.
    * 
    * @return uint8_t The value of the `row` field.
    */
    uint8_t get_row() const { return u.pt.row; }

    /**
    * @brief Getter for the `type` field of the `pt` and `ndx` structures.
    * 
    * @return uint8_t The value of the `type` field.
    */
    uint8_t get_type() const { return u.pt.type; }

    /**
    * @brief Getter for the `side` field of the `pt` and `ndx` structures.
    * 
    * @return uint8_t The value of the `side` field.
    */
    uint8_t get_side() const { return u.pt.side; }

};  // conv1_t


/**
 * @brief Struct representing a chess move.
 * 
 * The struct contains two `conv1_t` members, `from` and `to`, representing the
 * starting and ending positions of the move, respectively.
 */
struct conv2_t {
private:
    conv1_t   from, to;

public:
    /**
     * @brief Default constructor. Initializes both positions to (0, 0).
     */
    conv2_t() : from(), to() {}

    /**
     * @brief Constructor that takes the indices of the start and end positions.
     * 
     * @param from_index The index of the starting position.
     * @param to_index The index of the ending position.
     */
    conv2_t(uint8_t from_index, uint8_t to_index)
        : from(from_index), to(to_index) {}

    /**
     * @brief Constructor that takes the coordinates of the start and end positions.
     * 
     * @param from_col The column of the starting position.
     * @param from_row The row of the starting position.
     * @param to_col The column of the ending position.
     * @param to_row The row of the ending position.
     */
    conv2_t(uint8_t from_col, uint8_t from_row, uint8_t to_col, uint8_t to_row)
        : from(from_col, from_row), to(to_col, to_row) {}

    /**
     * @brief Constructor that takes two `conv1_t` objects to represent the start and end positions.
     * 
     * @param from_ The starting position.
     * @param to_ The ending position.
     */
    conv2_t(const conv1_t& from_, const conv1_t& to_)
        : from(from_), to(to_) {}

    void set_from_index(uint8_t value) { from.set_index(value); }
    void set_from_col(uint8_t value) { from.set_col(value); }
    void set_from_row(uint8_t value) { from.set_row(value); }
    void set_from_type(uint8_t value) { from.set_type(value); }
    void set_from_side(uint8_t value) { from.set_side(value); }

    void set_to_index(uint8_t value) { to.set_index(value); }
    void set_to_col(uint8_t value) { to.set_col(value); }
    void set_to_row(uint8_t value) { to.set_row(value); }
    void set_to_type(uint8_t value) { to.set_type(value); }
    void set_to_side(uint8_t value) { to.set_side(value); }

    uint8_t get_from_index() const { return from.get_index(); }
    uint8_t get_from_col() const { return from.get_col(); }
    uint8_t get_from_row() const { return from.get_row(); }
    uint8_t get_from_type() const { return from.get_type(); }
    uint8_t get_from_side() const { return from.get_side(); }

    uint8_t get_to_index() const { return to.get_index(); }
    uint8_t get_to_col() const { return to.get_col(); }
    uint8_t get_to_row() const { return to.get_row(); }
    uint8_t get_to_type() const { return to.get_type(); }
    uint8_t get_to_side() const { return to.get_side(); }

};  // conv2_t

#endif  // CONV_INCL