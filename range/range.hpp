#include <coroutine>
#include <optional>

namespace rbelli
{

template<class T>
class RangeRet;

class RangeIteratorEnd
{
};

template<class T>
class RangeIterator
{
private:
    RangeIterator(RangeRet<T> &range):range(range)
    {
        ++*this;
    }
    
    friend RangeRet<T>;

    RangeRet<T> &range;
    bool isEnd;
    T val_zero{};
public:
    T& operator*()
    {
        return value.has_value()?value.value():val_zero;
    }

    std::optional<T> value;

    RangeIterator<T>& operator++()
    {
        value = range.nextValue();
        return *this;
    }

    bool operator==(const RangeIteratorEnd &v)
    {
        return !value.has_value();
    }
    bool operator!=(const RangeIteratorEnd &v)
    {
        return value.has_value();
    }
};

template<class T>
class Promise;

/**
 * T <- Retorno do co_yield
 */
template<class T>
class RangeRet
{
public:
    using promise_type = Promise<T>;
    using handle_type = std::coroutine_handle<promise_type>;
private:
    handle_type handle;
    RangeRet(handle_type handle):handle(handle){}
    //
    // Deletado os o construtor de cópia e o operador de cópia. Para não ter 
    // duas instância do RangeRet com o mesmo handle,
    // se quiser duas instâncias com o mesmo handle, usar contador de usos, com shared_ptr por exemplo...
    // pois no final do uso o handle precisa ser destruido apenas uma vez.
    RangeRet(const RangeRet &) = delete; 
    RangeRet &operator=(const RangeRet&)=delete; 
public:
    RangeRet()
    {
        handle=nullptr;
    }
    // Construtor de move, este pode, troca quem tem o handle
    RangeRet(RangeRet &&h)
    {
        this->handle = h.handle;
        h.handle = nullptr;
    }
    void destroy()
    {
        handle.destroy();
        handle = nullptr;
    }

    // operator move...
    RangeRet &operator=(RangeRet &&h)
    {
        if(handle)
        {
            handle.destroy();
        }
        handle = h.handle;
        h.handle = nullptr;
        return *this;
    }


    //Destrutor, precisa chamar o destroy do handle
    // O destroy do handle só pode ser chamado uma vez, 
    //por isso, nesta implementação, não permitimos copia do RangeRet
    ~RangeRet()
    {
        if(handle)
        {
            handle.destroy();
        }
    }
    std::optional<T> nextValue()
    {
        Promise<T> &promise = handle.promise();
        promise.yieldedValue.reset();

        if(promise.inner.handle)
        {
            auto v = promise.inner.nextValue();
            if(v.has_value())
            {
                return v;
            }
            promise.inner.handle.destroy();
            promise.inner.handle = nullptr;
        }

        handle.resume();
        return promise.yieldedValue;
    }
    friend Promise<T>;

    RangeIterator<T> begin()
    {
        return RangeIterator<T>(*this);
    }
    RangeIteratorEnd end()
    {
        return RangeIteratorEnd{};
    }
};

/**
 * T <- Retorno do co_yield
 */
template<class T>
class Promise
{
public:
    using handle_type = std::coroutine_handle<Promise<T>>;
    RangeRet<T> get_return_object()
    {
        auto handle = handle_type::from_promise(*this);
        return RangeRet{handle};
    }
    /// @brief Se vai suspender quando a função for começar a executar
    /// @return retorna suspend_never, assim não suspende no inicio
    std::suspend_always initial_suspend() { return {}; }

    /// @brief 
    /// @return 
    std::suspend_always final_suspend() noexcept { return {}; }

    /// @brief Guardando o valor recebido pelo co_yield
    std::optional<T> yieldedValue;

    /// @brief função que trata o co_yield
    /// @param value parametro recebido pelo co_yield
    /// @return Se suspende ou não a execução
    std::suspend_always yield_value(T value)
    {
        yieldedValue = value;
        return {};
    }

    void unhandled_exception() {}

    /// @brief função que trata o co_await...
    /// @param  Se tiver parametro, é o parametro recebido pelo co_await
    /// @return Se suspende ou não a execução depois do co_await
    std::suspend_always await_transform() // transforma o retorno do co_await para o tipo de suspend_aways, or suspend_never or... // retorna um awaiter type
    {
        return {};
    }

    RangeRet<T> inner;

    std::suspend_always await_transform(RangeRet<T>  r)
    {
        yieldedValue = r.nextValue();
        if(!yieldedValue.has_value())
        {
            handle_type::from_promise(*this).resume();
        }
        else
        {
            inner = std::move(r);
        }

        return {};
    }

    /// @brief Função implementado, quando não a parâmetro retornado pelo co_return
    void return_void() {}

    // Se for tratar o co_return com valor, necessário implementar esta função abaixo:
    //
    //void return_value(std::string val)
    //{
    //    returned_value = val;
    //}
};


}