pragma solidity ^0.4.24;

contract BetEth {
    using SafeMath for *;

    mapping(uint256 => mapping(uint256 => bet)) public betpot_;
    mapping(uint256 => uint256) public id_;
    struct bet {
        address player1;
        address player2;
        address winner;
        uint8 result;
    }

    modifier isHuman() {
        address _addr = msg.sender;
        require(_addr == tx.origin);
        
        uint256 _codeLength;
        
        assembly {_codeLength := extcodesize(_addr)}
        require(_codeLength == 0, "sorry humans only");
        _;
    }

    constructor()
    public
    {
    }

    event sendlog(address,address,address,uint,uint8,uint);
    function betting()
    public
    isHuman
    payable
    {
        uint id = id_[msg.value];
        if (betpot_[msg.value][id].player1 == address(0x0)) {
            betpot_[msg.value][id].player1 = msg.sender;
        } else if (betpot_[msg.value][id].player2 == address(0x0)) {
            betpot_[msg.value][id].player2 = msg.sender;
            uint8 result = randomX();
            betpot_[msg.value][id].result = result;
            uint ret = (msg.value).mul(198).div(100);
            if (result < 5) {
                betpot_[msg.value][id].player1.transfer(ret);
                betpot_[msg.value][id].winner = betpot_[msg.value][id].player1;
            } else {
                betpot_[msg.value][id].player2.transfer(ret);
                betpot_[msg.value][id].winner = betpot_[msg.value][id].player2;
            }
            emit sendlog(betpot_[msg.value][id].player1, betpot_[msg.value][id].player2, betpot_[msg.value][id].winner,msg.value, result, now);
        } else {
            id++;
            betpot_[msg.value][id].player1 = msg.sender;
            id_[msg.value] = id;
        }
    }

    function randomX()
    private
    view
    returns (uint8)
    {
        uint256 x = uint256(keccak256(abi.encodePacked(
                (block.timestamp).add
                (block.difficulty).add
                ((uint256(keccak256(abi.encodePacked(block.coinbase)))) / (now)).add
                ((uint256(keccak256(abi.encodePacked(msg.sender)))) / (now)).add
                (block.number).add
                (gasleft()).add
                (block.gaslimit)
            )));        
        x = x - ((x / 10) * 10);
        return uint8(x);
    }
}

/**
 * @title SafeMath v0.1.9
 * @dev Math operations with safety checks that throw on error
 * change notes:  original SafeMath library from OpenZeppelin modified by Inventor
 * - added sqrt
 * - added sq
 * - added pwr 
 * - changed asserts to requires with error log outputs
 * - removed div, its useless
 */
library SafeMath {
    
    /**
    * @dev Multiplies two numbers, throws on overflow.
    */
    function mul(uint256 a, uint256 b) 
        internal 
        pure 
        returns (uint256 c) 
    {
        if (a == 0) {
            return 0;
        }
        c = a * b;
        require(c / a == b, "SafeMath mul failed");
        return c;
    }

    /**
    * @dev Integer division of two numbers, truncating the quotient.
    */
    function div(uint256 a, uint256 b) internal pure returns (uint256) {
        // assert(b > 0); // Solidity automatically throws when dividing by 0
        uint256 c = a / b;
        // assert(a == b * c + a % b); // There is no case in which this doesn't hold
        return c;
    }
    
    /**
    * @dev Subtracts two numbers, throws on overflow (i.e. if subtrahend is greater than minuend).
    */
    function sub(uint256 a, uint256 b)
        internal
        pure
        returns (uint256) 
    {
        require(b <= a, "SafeMath sub failed");
        return a - b;
    }

    /**
    * @dev Adds two numbers, throws on overflow.
    */
    function add(uint256 a, uint256 b)
        internal
        pure
        returns (uint256 c) 
    {
        c = a + b;
        require(c >= a, "SafeMath add failed");
        return c;
    }
    
    /**
     * @dev gives square root of given x.
     */
    function sqrt(uint256 x)
        internal
        pure
        returns (uint256 y) 
    {
        uint256 z = ((add(x,1)) / 2);
        y = x;
        while (z < y) 
        {
            y = z;
            z = ((add((x / z),z)) / 2);
        }
    }
    
    /**
     * @dev gives square. multiplies x by x
     */
    function sq(uint256 x)
        internal
        pure
        returns (uint256)
    {
        return (mul(x,x));
    }
    
    /**
     * @dev x to the power of y 
     */
    function pwr(uint256 x, uint256 y)
        internal 
        pure 
        returns (uint256)
    {
        if (x==0)
            return (0);
        else if (y==0)
            return (1);
        else 
        {
            uint256 z = x;
            for (uint256 i=1; i < y; i++)
                z = mul(z,x);
            return (z);
        }
    }
}