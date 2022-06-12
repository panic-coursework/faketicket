interface Success {
  success: true
}
interface User {
  username: string
  name: string
  email: string
  privilege: number
}
interface Train {
  // TODO
}
interface BuyTicketSuccess {
  status: 'success'
  price: number
}
interface BuyTicketEnqueued {
  status: 'enqueued'
}
type BuyTicketResponse = BuyTicketSuccess | BuyTicketEnqueued
interface Order {
  id: number
  trainId: string
  from: Date
  to: Date
  price: number
  seats: number
  subTotal: number
}

export type Response =
  Success |
  User |
  Train |
  Train[] |
  BuyTicketResponse |
  Order[]
